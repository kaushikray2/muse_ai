#!/usr/bin/env python3
"""Generate arduino-minimal-rev8.kicad_pcb — 2-layer PCB for the minimal ATmega328P-AU.

Board: 38 x 32mm, 2-layer, 1.6mm.
- F.Cu: VCC + signal tracks, SMD components.
- B.Cu: GND copper pour (vias from all GND pads).
- Track: 0.5mm power, 0.25mm signal. Via 0.8/0.4mm. Clearance 0.2mm.

Footprints are embedded from the installed KiCad 10 libraries.
Pad coordinates are used as-authored (KiCad pcbnew Y-down interpretation).
"""
import os, re, uuid, math

HERE = os.path.dirname(os.path.abspath(__file__))
SCH = os.path.join(HERE, "arduino-minimal-rev8.kicad_sch")
OUT = os.path.join(HERE, "arduino-minimal-rev8.kicad_pcb")
FP_DIR = "/usr/share/kicad/footprints"

def new_uuid():
    return str(uuid.uuid4())

def n(v):
    if abs(v - round(v)) < 1e-9:
        return str(int(round(v)))
    s = f"{v:.4f}".rstrip("0").rstrip(".")
    return s if s else "0"

# ---------------------------------------------------------------- nets
# net number -> name (must match schematic netlist)
NETS = {
    0: "",
    1: "VCC",
    2: "GND",
    3: "/RX",
    4: "/TX",
    5: "Net-(U1-AREF)",
    6: "Net-(U1-~{RESET}{slash}PC6)",
}
NET_BY_NAME = {v: k for k, v in NETS.items()}

# ref.pad -> net number
CONN = {
    # VCC net
    ("J2","1"): 1, ("U1","4"): 1, ("U1","6"): 1, ("C1","1"): 1,
    ("R1","1"): 1, ("U1","18"): 1, ("C2","1"): 1,
    # GND net
    ("J2","2"): 2, ("U1","3"): 2, ("U1","5"): 2, ("U1","21"): 2,
    ("C1","2"): 2, ("C2","2"): 2, ("C3","2"): 2, ("J1","3"): 2,
    # RX / TX
    ("J1","1"): 3, ("U1","30"): 3,
    ("J1","2"): 4, ("U1","31"): 4,
    # AREF / RESET
    ("C3","1"): 5, ("U1","20"): 5,
    ("R1","2"): 6, ("U1","29"): 6,
}

# ---------------------------------------------------------------- footprint sources
FP_FILES = {
    "U1": "Package_QFP.pretty/TQFP-32_7x7mm_P0.8mm.kicad_mod",
    "R1": "Resistor_SMD.pretty/R_0805_2012Metric.kicad_mod",
    "C1": "Capacitor_SMD.pretty/C_0805_2012Metric.kicad_mod",
    "C2": "Capacitor_SMD.pretty/C_0805_2012Metric.kicad_mod",
    "C3": "Capacitor_SMD.pretty/C_0805_2012Metric.kicad_mod",
    "J1": "Connector_PinHeader_2.54mm.pretty/PinHeader_1x03_P2.54mm_Vertical_SMD_Pin1Left.kicad_mod",
    "J2": "Connector_PinHeader_2.54mm.pretty/PinHeader_1x02_P2.54mm_Vertical_SMD_Pin1Left.kicad_mod",
}
FP_LIBID = {
    "U1": "Package_QFP:TQFP-32_7x7mm_P0.8mm",
    "R1": "Resistor_SMD:R_0805_2012Metric",
    "C1": "Capacitor_SMD:C_0805_2012Metric",
    "C2": "Capacitor_SMD:C_0805_2012Metric",
    "C3": "Capacitor_SMD:C_0805_2012Metric",
    "J1": "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical_SMD_Pin1Left",
    "J2": "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical_SMD_Pin1Left",
}
# placement: ref -> (x, y, rotation_deg)
# Y-DOWN (pcbnew convention). U1 signal pads (29/30/31) are at top (y=11.75).
PLACE = {
    "U1": (19.0, 16.0, 0),
    "C1": (10.5, 16.0, 0),
    "C2": (27.5, 18.0, 0),
    "C3": (27.5, 14.0, 0),
    "R1": (12.0, 7.0, 0),
    "J1": (34.0, 10.0, 0),
    "J2": (4.0, 16.0, 0),
}
REFDES_VALUE = {
    "U1": "ATmega328P-AU", "R1": "10k", "C1": "100nF", "C2": "100nF",
    "C3": "100nF", "J1": "Conn_01x03", "J2": "Conn_01x02",
}

def load_footprint(path):
    """Return (header_lines, pad_blocks) parsed from a .kicad_mod."""
    src = open(os.path.join(FP_DIR, path)).read()
    # strip outer (footprint ...) wrapper; keep inner content
    m = re.match(r'\(\s*footprint\s+"[^"]*"\s*(.*)\)\s*$', src, re.S)
    assert m, f"bad footprint {path}"
    inner = m.group(1)
    # split out pad blocks
    pads = []
    def pad_repl(mo):
        pads.append(mo.group(0))
        return ""
    inner_nopads = re.sub(r'\(pad\s+"[^"]*"\s+.*?(?=\(pad\s+"|\Z)', pad_repl, inner, flags=re.S)
    # simpler: find all (pad ...) balanced
    pads = []
    for mo in re.finditer(r'\(pad\s', inner):
        s = mo.start(); depth = 0; i = s; nn = len(inner)
        while i < nn:
            c = inner[i]
            if c == '(': depth += 1
            elif c == ')':
                depth -= 1
                if depth == 0: break
            elif c == '"': i = inner.index('"', i + 1)
            i += 1
        pads.append(inner[s:i + 1])
    # remove pads from inner to get the non-pad content; also drop the
    # library's own fp_text reference/value (we add our own below)
    rest = inner
    for p in pads:
        rest = rest.replace(p, "", 1)
    # remove balanced (fp_text ...) blocks for reference/value/user
    out = []
    i = 0
    nn = len(rest)
    while i < nn:
        m = re.match(r'\(fp_text\s+(reference|value|user)\b', rest[i:])
        if m:
            s = i; depth = 0; j = s
            while j < nn:
                c = rest[j]
                if c == '(': depth += 1
                elif c == ')':
                    depth -= 1
                    if depth == 0: break
                elif c == '"': j = rest.index('"', j + 1)
                j += 1
            i = j + 1
        else:
            out.append(rest[i]); i += 1
    rest = "".join(out)
    # drop footprint-file-only headers not valid inside a board
    rest = re.sub(r'\(version\s+[^)]+\)', '', rest)
    rest = re.sub(r'\(generator\s+[^)]+\)', '', rest)
    return rest, pads

def pad_number(pad_block):
    m = re.match(r'\(pad\s+"([^"]+)"', pad_block)
    return m.group(1)

def pad_at(pad_block):
    m = re.search(r'\(at\s+([-\d.]+)\s+([-\d.]+)', pad_block)
    return (float(m.group(1)), float(m.group(2)))

# ---------------------------------------------------------------- build
L = []
def emit(s=""):
    L.append(s)

emit("(kicad_pcb (version 20221018) (generator pcbnew)")
emit('  (general (thickness 1.6))')
emit('  (paper "A4")')
emit('  (paper "A4")')
emit('  (layers')
emit('    (0 "F.Cu" signal)')
emit('    (31 "B.Cu" signal)')
emit('    (32 "B.Adhes" user "B.Adhesive")')
emit('    (33 "F.Adhes" user "F.Adhesive")')
emit('    (34 "B.Paste" user)')
emit('    (35 "F.Paste" user)')
emit('    (36 "B.SilkS" user "B.Silkscreen")')
emit('    (37 "F.SilkS" user "F.Silkscreen")')
emit('    (38 "B.Mask" user)')
emit('    (39 "F.Mask" user)')
emit('    (40 "Dwgs.User" user "User.Drawings")')
emit('    (41 "Cmts.User" user "User.Comments")')
emit('    (42 "Eco1.User" user "User.Eco1")')
emit('    (43 "Eco2.User" user "User.Eco2")')
emit('    (44 "Edge.Cuts" user)')
emit('  )')
emit('  (setup')
emit('    (pad_to_mask_clearance 0)')
emit('    (pcbplotparams (layerselection 0x00010fc_ffffffff) (plot_on_all_layers_selection 0x0000000_00000000))')
emit('  )')
for num in sorted(NETS):
    emit(f'  (net {num} "{NETS[num]}")')

# ---------------- footprints
# pad absolute positions for routing: ref -> {padnum: (x, y)}
PADPOS = {}

for ref in ["U1", "R1", "C1", "C2", "C3", "J1", "J2"]:
    rest, pads = load_footprint(FP_FILES[ref])
    X, Y, rot = PLACE[ref]
    tstamp = new_uuid()
    emit(f'  (footprint "{FP_LIBID[ref]}" (layer "F.Cu") (tstamp {tstamp})')
    emit(f'    (at {n(X)} {n(Y)})')
    # carry over descriptive content (descr, tags, properties, graphics, models)
    # but drop the original (at ...) if present in rest (it isn't; at is separate)
    for line in rest.splitlines():
        s = line.strip()
        if not s:
            continue
        if re.match(r'\(at\s', s):
            continue
        # rewrite fp_text positions: keep as-is (relative)
        emit("    " + s if s else "")
    # reference/value texts
    emit(f'    (fp_text reference "{ref}" (at 0 -6.5) (layer "F.SilkS") (effects (font (size 1 1) (thickness 0.15))))')
    emit(f'    (fp_text value "{REFDES_VALUE[ref]}" (at 0 6.5) (layer "F.Fab") (effects (font (size 1 1) (thickness 0.15))))')
    PADPOS[ref] = {}
    for p in pads:
        num = pad_number(p)
        lx, ly = pad_at(p)
        # rotation (only 0 used, but handle generally; pcb Y-down, clockwise)
        if rot:
            r = math.radians(rot)
            lx, ly = (lx * math.cos(r) - ly * math.sin(r),
                      lx * math.sin(r) + ly * math.cos(r))
        ax, ay = X + lx, Y + ly
        PADPOS[ref][num] = (ax, ay)
        net = CONN.get((ref, num), 0)
        # inject (net N "name") before (tstamp, the canonical position
        if "(tstamp" in p:
            p2 = p.replace("(tstamp", f'(net {net} "{NETS[net]}") (tstamp', 1)
        else:
            p2 = p.rstrip()
            assert p2.endswith(")")
            p2 = p2[:-1] + f' (net {net} "{NETS[net]}")' + ")"
        for pl in p2.splitlines():
            emit("    " + pl if pl.strip() else "")
    emit("  )")

def P(ref, num):
    return PADPOS[ref][num]

# ---------------- tracks & vias
def seg(x1, y1, x2, y2, net, layer="F.Cu", width=0.25):
    emit(f'  (segment (start {n(x1)} {n(y1)}) (end {n(x2)} {n(y2)}) '
         f'(width {width}) (layer "{layer}") (net {net}) (tstamp {new_uuid()}))')

def via(x, y, net, size=0.8, drill=0.4):
    emit(f'  (via (at {n(x)} {n(y)}) (size {size}) (drill {drill}) '
         f'(layers "F.Cu" "B.Cu") (net {net}) (tstamp {new_uuid()}))')

def poly_track(pts, net, layer="F.Cu", width=0.25):
    for (x1, y1), (x2, y2) in zip(pts, pts[1:]):
        seg(x1, y1, x2, y2, net, layer, width)

VCC, GND, RX, TX, AREF, RESET = 1, 2, 3, 4, 5, 6
W_PWR, W_SIG = 0.5, 0.25

# ---- VCC (F.Cu, 0.5mm)
# J2.1 -> C1.1 -> bus -> U1 P4/P6
p = P("J2","1"); q = P("C1","1")
poly_track([(p[0],p[1]), (p[0],16.0), (q[0],16.0)], VCC, "F.Cu", W_PWR)
poly_track([(12.5,16.0), (12.5,15.6), P("U1","4")], VCC, "F.Cu", W_PWR)
poly_track([(12.5,16.0), (12.5,17.2), P("U1","6")], VCC, "F.Cu", W_PWR)
# R1.1 -> VCC bus: jog around C1.2's GND pad (center 11.45,16.0;
# a straight x=p[0] run would short VCC to that pad). The bus runs at
# y=14.5 (clears C1.2's bottom edge at 15.275 by 0.5mm+) with the riser
# at x=12.5 (clears C1.2's right edge at 11.95 by 0.3mm). C1.1 joins the
# same bus by dropping straight down from its pad.
p = P("R1","1")
poly_track([(p[0],p[1]), (p[0],14.5)], VCC, "F.Cu", W_PWR)
poly_track([(q[0],q[1]), (q[0],14.5), (p[0],14.5), (12.5,14.5), (12.5,16.0)],
           VCC, "F.Cu", W_PWR)
# AVCC: U1.18 -> C2.1 -> around (y=21.75, clears the unconnected U1 pads
# whose tops reach y=21.05) to VCC bus
p18 = P("U1","18"); c2 = P("C2","1")
poly_track([p18, (c2[0],p18[1]), (c2[0],21.75), (12.5,21.75), (12.5,17.2)], VCC, "F.Cu", W_PWR)

# ---- GND: vias to B.Cu pour
def gnd_via(x, y, size=0.8, drill=0.4):
    via(x, y, GND, size=size, drill=drill)
# U1 P3, P5, P21 -- note: the P3/P5 vias are 0.6/0.3, not 0.8/0.4: they sit
# between the VCC stubs to U1.4 (y=15.6) and U1.6 (y=17.2) on the 0.8mm pad
# pitch, where a 0.8 via cannot meet 0.2mm clearance (needs 0.85, has 0.8)
for (pad, vx), vsz in [((P("U1","3"), 13.5), (0.6, 0.3)),
                       ((P("U1","5"), 13.5), (0.6, 0.3)),
                       ((P("U1","21"), 24.5), (0.8, 0.4))]:
    seg(pad[0], pad[1], vx, pad[1], GND, "F.Cu", W_SIG)
    gnd_via(vx, pad[1], size=vsz[0], drill=vsz[1])
# C1.2, C2.2, C3.2 -- C3.2's via goes UP to y=14.5 so it clears the TX B.Cu
# run at y=12
for ref, vy in [("C1", 17.5), ("C2", 19.5), ("C3", 14.5)]:
    pad = P(ref, "2")
    seg(pad[0], pad[1], pad[0], vy, GND, "F.Cu", W_SIG)
    gnd_via(pad[0], vy)
# J1.3, J2.2
pad = P("J1","3")
seg(pad[0], pad[1], 33.5, pad[1], GND, "F.Cu", W_SIG)
seg(33.5, pad[1], 33.5, 14.0, GND, "F.Cu", W_SIG)
gnd_via(33.5, 14.0)
pad = P("J2","2")
seg(pad[0], pad[1], pad[0], 19.0, GND, "F.Cu", W_SIG)
gnd_via(pad[0], 19.0)

# ---- RESET: R1.2 -> U1.29 (F.Cu, 0.25); horizontal at y=9.5 to clear
# the RX via at (17.8,10.5)
p = P("R1","2"); q = P("U1","29")
poly_track([(p[0],p[1]), (p[0],9.5), (q[0],9.5), (q[0],q[1])], RESET, "F.Cu", W_SIG)

# ---- RX: J1.1 -> U1.30 via B.Cu (F.Cu stubs + vias)
p = P("J1","1"); q = P("U1","30")
seg(p[0], p[1], 30.0, p[1], RX, "F.Cu", W_SIG)
via(30.0, p[1], RX)
seg(30.0, p[1], 30.0, 10.5, RX, "B.Cu", W_SIG)
seg(30.0, 10.5, q[0], 10.5, RX, "B.Cu", W_SIG)
via(q[0], 10.5, RX)
seg(q[0], 10.5, q[0], q[1], RX, "F.Cu", W_SIG)

# ---- TX: J1.2 -> U1.31 via B.Cu; the F.Cu-side via sits at y=13 (not
# y=12) so it clears the neighbouring U1.30 (RX) and U1.32 (NC) pads
p = P("J1","2"); q = P("U1","31")
seg(p[0], p[1], 33.0, p[1], TX, "F.Cu", W_SIG)
via(33.0, p[1], TX)
seg(33.0, p[1], 33.0, 12.0, TX, "B.Cu", W_SIG)
seg(33.0, 12.0, q[0], 12.0, TX, "B.Cu", W_SIG)
seg(q[0], 12.0, q[0], 13.0, TX, "B.Cu", W_SIG)
via(q[0], 13.0, TX)
seg(q[0], 13.0, q[0], q[1], TX, "F.Cu", W_SIG)

# ---- AREF: C3.1 -> U1.20 (F.Cu, 0.25)
p = P("C3","1"); q = P("U1","20")
poly_track([(p[0],p[1]), (p[0],q[1]), (q[0],q[1])], AREF, "F.Cu", W_SIG)

# ---- B.Cu GND pour
emit(f'  (zone (net {GND}) (net_name "GND") (layers "B.Cu") (tstamp {new_uuid()})')
emit('    (name "GND-pour")')
emit('    (hatch edge 0.5)')
emit('    (connect_pads (clearance 0.5))')
emit('    (min_thickness 0.25)')
emit('    (filled_areas_thickness no)')
emit('    (fill (thermal_gap 0.5) (thermal_bridge_width 0.5))')
emit('    (polygon (pts (xy 0.5 0.5) (xy 37.5 0.5) (xy 37.5 31.5) (xy 0.5 31.5)))')
emit('  )')

# ---- Edge.Cuts
emit('  (gr_rect (start 0 0) (end 38 32) (layer "Edge.Cuts") (width 0.1) (tstamp {}))'.format(new_uuid()))

emit(")")

with open(OUT, "w") as f:
    f.write("\n".join(L) + "\n")
print(f"wrote {OUT} ({os.path.getsize(OUT)} bytes)")
