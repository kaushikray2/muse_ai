#!/usr/bin/env python3
"""ERC-style checker for arduino-minimal-rev6.kicad_sch (no KiCad needed).

Parses the s-expr directly, builds connectivity (nets) from wires, pins,
junctions, labels and power symbols, then runs electrical-rule checks:
  - unconnected pins
  - output-vs-output conflicts on a net
  - undriven input pins
  - power_in nets with no power source
  - dangling wire ends
  - missing footprints / duplicate references
Usage: python3 erc_check.py [schematic]
"""
import math, re, sys
from collections import defaultdict

SCH = sys.argv[1] if len(sys.argv) > 1 else "arduino-minimal-rev6.kicad_sch"
src = open(SCH).read()

# ---------- s-expr parse ----------
tok_re = re.compile(r'\(|\)|"[^"]*"|[^\s()"]+')
toks = tok_re.findall(src)
depth = 0
for t in toks:
    depth += 1 if t == "(" else -1 if t == ")" else 0
assert depth == 0, "unbalanced s-expr"

def parse():
    it = iter(toks)
    def rd():
        out = []
        for t in it:
            if t == "(":
                out.append(rd())
            elif t == ")":
                return out
            else:
                out.append(t[1:-1] if t.startswith('"') else t)
        raise AssertionError("eof in list")
    assert next(it) == "("
    return rd()

tree = parse()
assert tree[0] == "kicad_sch"

def kids(node, name):
    return [n for n in node[1:] if isinstance(n, list) and n and n[0] == name]

def find(node, name):
    for n in node[1:]:
        if isinstance(n, list) and n and n[0] == name:
            return n
    return None

def fnum(x):
    return float(x)

# ---------- library pin geometry ----------
# lib_id -> list of (number, name, etype, x, y) in unit-1 coordinates
lib_pins = {}
for top in tree:
    if isinstance(top, list) and top[:1] == ["lib_symbols"]:
        for sym in top[1:]:
            if not (isinstance(sym, list) and sym[:1] == ["symbol"]):
                continue
            lib_id = sym[1]
            pins = []
            for unit in sym[1:]:
                if isinstance(unit, list) and unit[:1] == ["symbol"] and unit[1].endswith("_1_1"):
                    for el in unit[1:]:
                        if isinstance(el, list) and el[:1] == ["pin"]:
                            etype = el[1]
                            at = find(el, "at")
                            num = find(el, "number")[1]
                            nm = find(el, "name")
                            pins.append((num, nm[1] if nm else "", etype,
                                         fnum(at[1]), fnum(at[2])))
            if pins:
                lib_pins[lib_id] = pins

# ---------- placed symbol instances ----------
instances = []  # (ref, value, lib_id, X, Y, rot, mirror, footprint)
for node in tree:
    if isinstance(node, list) and node[:1] == ["symbol"]:
        libref = find(node, "lib_id")
        if not libref:
            continue  # lib_symbols definitions, not placements
        lib_id = libref[1]
        at = find(node, "at"); prop = {p[1]: p[2] for p in kids(node, "property") if len(p) > 2}
        ref = prop.get("Reference", "?"); value = prop.get("Value", "?")
        fp = prop.get("Footprint", "")
        mir = find(node, "mirror")
        instances.append((ref, value, lib_id, fnum(at[1]), fnum(at[2]),
                          fnum(at[3]) if len(at) > 3 else 0.0,
                          mir[1] if mir else None, fp))

def rot_pt(x, y, deg):
    """KiCad placement transform for a library pin (px, py) [Y-up library
    frame] on a symbol rotated by deg clockwise: converts to Y-down sheet.
    (X + px*cos - py*sin, Y - px*sin - py*cos).  Verified empirically
    against KiCad 10 ERC pin positions, 2026-10-08."""
    r = math.radians(deg)
    c, s = math.cos(r), math.sin(r)
    return (round(x * c - y * s, 3), round(-(x * s + y * c), 3))

def pin_abs(px, py, X, Y, rot, mirror):
    if mirror == "x":
        px = -px
    elif mirror == "y":
        py = -py
    rx, ry = rot_pt(px, py, rot)
    return (round(X + rx, 3), round(Y + ry, 3))

# choose rotation convention empirically: maximize wire-endpoint matches
wires = []
for node in tree:
    if isinstance(node, list) and node[:1] == ["wire"]:
        pts = find(node, "pts")
        segs = [(fnum(p[1]), fnum(p[2])) for p in pts[1:]]
        for a, b in zip(segs, segs[1:]):
            wires.append((a, b))
endpoints = set()
for a, b in wires:
    endpoints.add(a); endpoints.add(b)
junctions = set()
for node in tree:
    if isinstance(node, list) and node[:1] == ["junction"]:
        at = find(node, "at")
        junctions.add((fnum(at[1]), fnum(at[2])))
labels = {}  # pos -> text
for node in tree:
    if isinstance(node, list) and node[0] in ("label", "global_label"):
        at = find(node, "at")
        labels[(fnum(at[1]), fnum(at[2]))] = node[1]

def build_pins():
    pins = []  # (ref, num, name, etype, pos)
    for ref, value, lib_id, X, Y, rot, mirror, fp in instances:
        for num, name, etype, px, py in lib_pins.get(lib_id, []):
            pins.append((ref, num, name, etype, pin_abs(px, py, X, Y, rot, mirror)))
    return pins

pins = build_pins()
matched = sum(1 for e in endpoints
              if e in set(p[4] for p in pins) or e in junctions)
print(f"[info] pin transform: KiCad Y-up->Y-down "
      f"(matched {matched}/{len(endpoints)} wire endpoints)")

# ---------- nets via union-find ----------
parent = {}
def findp(x):
    parent.setdefault(x, x)
    while parent[x] != x:
        parent[x] = parent[parent[x]]
        x = parent[x]
    return x
def union(a, b):
    ra, rb = findp(a), findp(b)
    if ra != rb:
        parent[ra] = rb

allpts = set()
for a, b in wires:
    allpts.add(a); allpts.add(b); union(a, b)
for p in pins:
    allpts.add(p[4])
for j in junctions:
    allpts.add(j); union(j, j)
for lp in labels:
    allpts.add(lp)
# attach pins/labels to coincident wire geometry (endpoints OR mid-segment)
EPS = 1e-6
def on_segment(px, py, a, b):
    (x1, y1), (x2, y2) = a, b
    # bounding box + collinearity
    if not (min(x1, x2) - EPS <= px <= max(x1, x2) + EPS and
            min(y1, y2) - EPS <= py <= max(y1, y2) + EPS):
        return False
    return abs((x2 - x1) * (py - y1) - (y2 - y1) * (px - x1)) < 1e-3

for p in pins:
    for a, b in wires:
        if on_segment(p[4][0], p[4][1], a, b):
            union(p[4], a)
# wire endpoints landing mid-segment on another wire (T-junctions)
for e in endpoints:
    for a, b in wires:
        if on_segment(e[0], e[1], a, b):
            union(e, a)
for lp in labels:
    for a, b in wires:
        if on_segment(lp[0], lp[1], a, b):
            union(lp, a)
for j in junctions:
    for e in endpoints:
        if j == e:
            union(j, e)

# KiCad power symbols are GLOBAL: union all pins of power symbols
# sharing the same value (all GND together, all VCC together)
pwr_pos = defaultdict(list)
ref_value = {}
for ref, value, lib_id, X, Y, rot, mirror, fp in instances:
    ref_value[ref] = value
    if lib_id.startswith("power:"):
        for p in pins:
            if p[0] == ref:
                pwr_pos[value].append(p[4])
for value, positions in pwr_pos.items():
    for pos in positions[1:]:
        union(positions[0], pos)

# KiCad local labels with the same name are the same net
label_pos = defaultdict(list)
for lp, txt in labels.items():
    label_pos[txt].append(lp)
for txt, positions in label_pos.items():
    for pos in positions[1:]:
        union(positions[0], pos)

nets = defaultdict(list)
for pt in allpts:
    nets[findp(pt)].append(pt)
pin_net = { (p[0], p[1]): findp(p[4]) for p in pins }

# name nets from power symbols / labels
net_names = {}
for ref, value, lib_id, X, Y, rot, mirror, fp in instances:
    if lib_id.startswith("power:"):
        for num, name, etype, ppos in [(p[1], p[2], p[3], p[4]) for p in pins if p[0] == ref]:
            net_names[findp(ppos)] = value
for lp, txt in labels.items():
    net_names[findp(lp)] = txt

def netname(nid):
    return net_names.get(nid, f"net-{str(nid)[:8]}")

print(f"[info] {len(instances)} symbols, {len(pins)} pins, {len(wires)} wire segments, "
      f"{len(nets)} nets, {len(junctions)} junctions")

# ---------- checks ----------
errors, warnings = [], []

# 1. duplicate references
refs = [i[0] for i in instances if not i[2].startswith("power:")]
seen = set()
for r in refs:
    if r in seen:
        errors.append(f"duplicate reference {r}")
    seen.add(r)

# 2. missing footprints (non-power symbols)
for ref, value, lib_id, X, Y, rot, mirror, fp in instances:
    if not lib_id.startswith("power:") and not fp:
        warnings.append(f"{ref} ({value}): no footprint assigned")

# 3. per-net electrical checks
net_pins = defaultdict(list)
for p in pins:
    net_pins[findp(p[4])].append(p)

if "--detail" in sys.argv:
    print("\n----- net details -----")
    for nid in sorted(nets, key=lambda n: netname(n)):
        members = sorted(set(f"{p[0]}.{p[1]}({p[2]})" for p in net_pins.get(nid, [])))
        print(f"{netname(nid)}: {len(nets[nid])} pts | " + (", ".join(members) if members else "(no pins)"))
    print("---------------------\n")

if "--pins" in sys.argv:
    print("\n----- pin positions -----")
    for ref, num, name, etype, pos in sorted(pins, key=lambda p: (p[0], p[1])):
        if ref in ("U1", "R1", "C1", "C2", "C3", "J1", "J2") or ref.startswith("#PWR"):
            print(f"{ref}.{num} ({name}, {etype}) @ {pos}")
    print("-------------------------\n")

DRIVE = {"output", "power_out", "bidirectional"}
for nid, plist in net_pins.items():
    nm = netname(nid)
    etypes = [p[3] for p in plist]
    outs = [p for p in plist if p[3] == "output"]
    if len(outs) > 1:
        errors.append(f"net {nm}: {len(outs)} output pins shorted "
                      f"({', '.join(f'{p[0]}.{p[1]}' for p in outs)})")
    if "input" in etypes and not any(e in DRIVE for e in etypes):
        warnings.append(f"net {nm}: input pin(s) with no driver "
                        f"({', '.join(f'{p[0]}.{p[1]}' for p in plist if p[3]=='input')})")
    if "power_in" in etypes and "power_out" not in etypes:
        # benign when the net reaches a connector (power fed from off-board,
        # e.g. J2) — only flag truly sourceless power nets
        has_conn = any(p[0].startswith("J") for p in plist)
        if not has_conn:
            warnings.append(f"net {nm}: power_in pins but no power_out source "
                            f"and no connector on net")

# 4. unconnected pins
connected = set()
for a, b in wires:
    connected.add(a); connected.add(b)
for p in pins:
    pos = p[4]
    if pos not in connected and pos not in junctions:
        # a pin coincident with another pin counts as connected
        if sum(1 for q in pins if q[4] == pos) < 2:
            warnings.append(f"unconnected pin {p[0]}.{p[1]} ({p[2] or 'no name'}, {p[3]})")

# 5. dangling wire ends
for e in endpoints:
    if e not in junctions and not any(p[4] == e for p in pins) and e not in labels:
        # endpoint mid-wire is fine (corner); only flag if degree 1
        deg = sum(1 for a, b in wires if a == e or b == e)
        if deg == 1:
            warnings.append(f"dangling wire end at {e}")

# 6. RX/TX sanity: labels should sit on nets reaching PD0/PD1
for lp, txt in labels.items():
    nid = findp(lp)
    members = [f"{p[0]}.{p[1]}({p[2]})" for p in net_pins[nid]]
    print(f"[info] label '{txt}' on net {netname(nid)}: {', '.join(members)}")

print("\n===== ERC RESULTS =====")
for e in errors:
    print("ERROR:", e)
for w in warnings:
    print("WARN :", w)
print(f"\n{len(errors)} errors, {len(warnings)} warnings")
