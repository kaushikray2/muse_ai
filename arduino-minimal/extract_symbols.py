#!/usr/bin/env python3
"""Extract official KiCad symbols from .kicad_symdir files, sanitize for v9,
and emit them with fully-qualified lib ids for embedding in lib_symbols."""
import os, re, sys

REF = "/tmp/kicad-symbols"
# (source file, library nickname for lib_id prefix)
SYMBOLS = [
    ("MCU_Microchip_ATmega.kicad_symdir/ATmega48PV-10A.kicad_sym", "MCU_Microchip_ATmega"),
    ("Device.kicad_symdir/R.kicad_sym", "Device"),
    ("Device.kicad_symdir/C.kicad_sym", "Device"),
    ("Connector.kicad_symdir/Conn_01x03_Pin.kicad_sym", "Connector"),
    ("Connector.kicad_symdir/Conn_01x02_Pin.kicad_sym", "Connector"),
    ("power.kicad_symdir/GND.kicad_sym", "power"),
    ("power.kicad_symdir/VCC.kicad_sym", "power"),
]

STRIP_NODES = {"in_pos_files", "duplicate_pin_numbers_are_jumpers"}


def strip_nodes(src):
    """Remove top-level (within symbol) nodes whose head is in STRIP_NODES.
    Simple balanced-paren stripper operating on the symbol body text."""
    out = []
    i, n = 0, len(src)
    while i < n:
        m = re.match(r'\(\s*(in_pos_files|duplicate_pin_numbers_are_jumpers)\b', src[i:])
        if m and src[i] == '(':
            # check this '(' opens one of the strip nodes: find head token
            head = re.match(r'\(\s*([A-Za-z_][\w]*)', src[i:]).group(1)
            if head in STRIP_NODES:
                depth = 0
                j = i
                while j < n:
                    if src[j] == '(':
                        depth += 1
                    elif src[j] == ')':
                        depth -= 1
                        if depth == 0:
                            break
                    elif src[j] == '"':
                        j += 1
                        while j < n and src[j] != '"':
                            j += 1
                    j += 1
                i = j + 1
                continue
        out.append(src[i])
        i += 1
    return ''.join(out)


def extract(path, libnick):
    src = open(os.path.join(REF, path)).read()
    # find the top-level (symbol "NAME" ...) node: first occurrence after lib header
    m = re.search(r'\(symbol\s+"([^"]+)"', src)
    assert m, f"no symbol in {path}"
    name = m.group(1)
    start = m.start()
    # balance from start
    depth, j, n = 0, start, len(src)
    while j < n:
        c = src[j]
        if c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0:
                break
        elif c == '"':
            j += 1
            while j < n and src[j] != '"':
                j += 1
        j += 1
    body = src[start:j + 1]
    body = strip_nodes(body)
    # rename to qualified lib id
    body = body.replace(f'(symbol "{name}"', f'(symbol "{libnick}:{name}"', 1)
    # also rename inner unit symbols: (symbol "NAME_0_1" -> (symbol "libnick:NAME_0_1"
    # NOTE: unit sub-symbols (NAME_0_1, NAME_1_1) must KEEP their short names.
    # Only the top-level symbol gets the qualified lib id; KiCad rejects
    # qualified unit names like (symbol "Device:R_0_1") — "Failed to load schematic".
    for tok in STRIP_NODES:
        assert f'({tok}' not in body, f"{tok} survived in {name}"
    return f"{libnick}:{name}", name, body


if __name__ == "__main__":
    for path, libnick in SYMBOLS:
        lib_id, name, body = extract(path, libnick)
        print(f"{lib_id}: {len(body)} chars")
