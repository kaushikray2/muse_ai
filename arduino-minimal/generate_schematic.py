#!/usr/bin/env python3
"""Generate arduino-minimal-rev8.kicad_sch — minimal ATmega328P-AU board.

Uses official KiCad library symbols embedded in lib_symbols.
Internal 8MHz oscillator, no crystal.

Coordinate systems (critical):
  KiCad's symbol libraries are authored Y-UP, but the schematic sheet is
  Y-DOWN.  KiCad converts on placement: for a symbol at (X, Y) rotated by
  clockwise angle t, a library pin at (px, py) lands at
      (X + px*cos(t) - py*sin(t),  Y - px*sin(t) - py*cos(t))
  i.e. for t=0: (X+px, Y-py); for t=180: (X-px, Y+py).
  Wires MUST be routed to these converted positions, not to (X+px, Y+py).
  (Rev 6/7 wired to the unconverted positions; KiCad ERC reported every
  pin as unconnected.)
"""
import os, sys, uuid

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from extract_symbols import extract, SYMBOLS

REV = 8
PROJ = f"arduino-minimal-rev{REV}"
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), PROJ + ".kicad_sch")
PRO_OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), PROJ + ".kicad_pro")

import math

lines = []


def emit(s=""):
    lines.append(s)


def new_uuid():
    return str(uuid.uuid4())


def n(v):
    if abs(v - round(v)) < 1e-9:
        return str(int(round(v)))
    s = f"{v:.3f}".rstrip("0").rstrip(".")
    return s if s else "0"


# ---------------------------------------------------------------- pin data
# Library pin positions (x, y) in the library's Y-UP frame.
ATMEGA_PINS = {
    "1": ("PD3", 15.24, -20.32), "2": ("PD4", 15.24, -22.86),
    "3": ("GND", 0.0, -38.1), "4": ("VCC", 0.0, 38.1),
    "5": ("GND", 0.0, -38.1), "6": ("VCC", 0.0, 38.1),
    "7": ("XTAL1/PB6", 15.24, 15.24), "8": ("XTAL2/PB7", 15.24, 12.7),
    "9": ("PD5", 15.24, -25.4), "10": ("PD6", 15.24, -27.94),
    "11": ("PD7", 15.24, -30.48), "12": ("PB0", 15.24, 30.48),
    "13": ("PB1", 15.24, 27.94), "14": ("PB2", 15.24, 25.4),
    "15": ("PB3", 15.24, 22.86), "16": ("PB4", 15.24, 20.32),
    "17": ("PB5", 15.24, 17.78), "18": ("AVCC", 2.54, 38.1),
    "19": ("ADC6", -15.24, 25.4), "20": ("AREF", -15.24, 30.48),
    "21": ("GND", 0.0, -38.1), "22": ("ADC7", -15.24, 22.86),
    "23": ("PC0", 15.24, 7.62), "24": ("PC1", 15.24, 5.08),
    "25": ("PC2", 15.24, 2.54), "26": ("PC3", 15.24, 0.0),
    "27": ("PC4", 15.24, -2.54), "28": ("PC5", 15.24, -5.08),
    "29": ("RESET", 15.24, -7.62), "30": ("PD0", 15.24, -12.7),
    "31": ("PD1", 15.24, -15.24), "32": ("PD2", 15.24, -17.78),
}
R_PINS = {"1": (0.0, 3.81), "2": (0.0, -3.81)}
C_PINS = {"1": (0.0, 3.81), "2": (0.0, -3.81)}
# Conn_01x03_Pin / Conn_01x02_Pin: pins on +x side; we use 180 deg rotation.
J3_PINS = {"1": (5.08, 2.54), "2": (5.08, 0.0), "3": (5.08, -2.54)}
J2_PINS = {"1": (5.08, 0.0), "2": (5.08, -2.54)}

U1X, U1Y = 110.0, 105.0


def kicad_pin(x, y, rot_deg, px, py):
    """Sheet position of a library pin (px, py) [Y-up] on a symbol at
    (x, y) rotated by rot_deg clockwise.  Converts Y-up -> Y-down."""
    t = math.radians(rot_deg)
    c, s = math.cos(t), math.sin(t)
    return (x + px * c - py * s, y - px * s - py * c)


def apin(num):
    """Sheet position of a U1 pin (U1 at rot 0)."""
    _, px, py = ATMEGA_PINS[num]
    return kicad_pin(U1X, U1Y, 0, px, py)


def rpin(pin_dict, x, y, rot, num):
    """Sheet position of a pin on a symbol at (x, y) rotated by rot."""
    px, py = pin_dict[num]
    if rot not in (0, 180):
        raise ValueError("only 0/180 supported")
    return kicad_pin(x, y, rot, px, py)


# ---------------------------------------------------------------- header
emit("(kicad_sch")
emit('  (version 20250114)')
emit('  (generator "eeschema")')
emit('  (generator_version "9.0")')
emit(f'  (uuid "{new_uuid()}")')
emit('  (paper "A4")')
emit('  (title_block')
emit('    (title "Minimal Arduino (ATmega328P-AU, internal 8MHz)")')
emit('    (date "2026-10-08")')
emit(f'    (rev "{REV}")')
emit('    (company "SiliconBrane Inc")')
emit('    (comment 1 "UART: RX/TX/GND on J1; regulated +5V/GND in on J2")')
emit('    (comment 2 "Fuses: LF 0xE2 (8MHz int), HF 0xDA, EF 0x05")')
emit("  )")

# ---------------------------------------------------------------- lib_symbols
# NOTE: no version/generator lines inside lib_symbols (KiCad rejects them),
# and unit sub-symbols keep short names (extract_symbols handles both).
emit('  (lib_symbols')
for path, libnick in SYMBOLS:
    lib_id, name, body = extract(path, libnick)
    for bl in body.splitlines():
        emit("    " + bl if bl.strip() else "")
emit("  )")

# ---------------------------------------------------------------- instances
FONT = "(effects (font (size 1.27 1.27)))"


def prop(name, value, x, y, hide=False):
    h = " (hide yes)" if hide else ""
    # NOTE: no (unlocked) — not valid KiCad syntax, breaks loading.
    return (f'    (property "{name}" "{value}" (at {n(x)} {n(y)} 0){h} '
            f"{FONT})")


def instance(ref, lib_id, x, y, rot, value, footprint, datasheet,
             pin_nums, ref_at, val_at, hide_ref=False):
    emit(f'  (symbol (lib_id "{lib_id}") (at {n(x)} {n(y)} {rot}) (unit 1)')
    emit("    (exclude_from_sim no)")
    emit("    (in_bom yes)")
    emit("    (on_board yes)")
    emit(f'    (uuid "{new_uuid()}")')
    emit(prop("Reference", ref, *ref_at, hide=hide_ref))
    emit(prop("Value", value, *val_at))
    emit(prop("Footprint", footprint, x, y, hide=True))
    emit(prop("Datasheet", datasheet, x, y, hide=True))
    for pn in pin_nums:
        emit(f'    (pin "{pn}" (uuid "{new_uuid()}"))')
    emit(f'    (instances (project "{PROJ}" (path "/{ROOT_UUID}" '
         f'(reference "{ref}") (unit 1))))')
    emit("  )")


ROOT_UUID = new_uuid()

U_LIB = "MCU_Microchip_ATmega:ATmega48PV-10A"
U_PINS = [str(i) for i in range(1, 33)]
instance("U1", U_LIB, U1X, U1Y, 0, "ATmega328P-AU",
         "Package_QFP:TQFP-32_7x7mm_P0.8mm",
         "http://ww1.microchip.com/downloads/en/DeviceDoc/ATmega328_P%20AVR%20MCU%20with%20picoPower%20Technology%20Data%20Sheet%2040001984A.pdf",
         U_PINS, (U1X - 12.7, U1Y - 38.5), (U1X + 2.54, U1Y + 38.5))
instance("R1", "Device:R", 135, 80, 0, "10k",
         "Resistor_SMD:R_0805_2012Metric", "", ["1", "2"],
         (137, 75), (137, 85))
instance("C1", "Device:C", 90, 60, 0, "100nF",
         "Capacitor_SMD:C_0805_2012Metric", "", ["1", "2"],
         (92, 55), (92, 65))
instance("C2", "Device:C", 125, 60, 0, "100nF",
         "Capacitor_SMD:C_0805_2012Metric", "", ["1", "2"],
         (127, 55), (127, 65))
instance("C3", "Device:C", 80, 80, 0, "100nF",
         "Capacitor_SMD:C_0805_2012Metric", "", ["1", "2"],
         (82, 75), (82, 85))
instance("J1", "Connector:Conn_01x03_Pin", 145, 118, 180, "Conn_01x03",
         "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical_SMD",
         "", ["1", "2", "3"], (147, 113), (147, 123))
instance("J2", "Connector:Conn_01x02_Pin", 60, 158, 180, "Conn_01x02",
         "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical_SMD",
         "", ["1", "2"], (62, 153), (62, 163))
# power symbols
pwr = 0


def power(sym, x, y):
    global pwr
    pwr += 1
    lib = "power:VCC" if sym == "VCC" else "power:GND"
    instance(f"#PWR{pwr:02d}", lib, x, y, 0, sym, "", "", ["1"],
             (x + 2, y), (x + 2, y), hide_ref=True)


# ---------------------------------------------------------------- wiring
# Layout: VCC bus along the top (y=40), GND symbols below components.
# All coordinates computed via kicad_pin() so wires meet KiCad's pins.
BUS_Y = 40.0


def wire(x1, y1, x2, y2):
    emit(f'  (wire (pts (xy {n(x1)} {n(y1)}) (xy {n(x2)} {n(y2)})) '
         f'(stroke (width 0) (type default)) (uuid "{new_uuid()}"))')


def poly(*pts):
    for (x1, y1), (x2, y2) in zip(pts, pts[1:]):
        wire(x1, y1, x2, y2)


def junction(x, y):
    emit(f'  (junction (at {n(x)} {n(y)}) (diameter 0) '
         f'(uuid "{new_uuid()}"))')


def label(text, x, y):
    emit(f'  (label "{text}" (at {n(x)} {n(y)} 0) '
         f'(effects (font (size 1.27 1.27)) (justify left)) '
         f'(uuid "{new_uuid()}"))')


def noconnect(x, y):
    emit(f'  (no_connect (at {n(x)} {n(y)}) (uuid "{new_uuid()}"))')


# VCC bus: J2 pin 1 up the left side, then across the top
J2_P1 = rpin(J2_PINS, 60, 158, 180, "1")          # (54.92, 158)
wire(*J2_P1, 54.92, BUS_Y)
wire(54.92, BUS_Y, 135, BUS_Y)
power("VCC", 70, BUS_Y)
junction(70, BUS_Y)                                  # VCC symbol tap
junction(90, BUS_Y)                                  # C1 tap
junction(110, BUS_Y)                                 # U1 VCC tap
junction(112.54, BUS_Y)                              # U1 AVCC tap
junction(125, BUS_Y)                                 # C2 tap
junction(135, BUS_Y)                                 # R1 tap

# U1 VCC / AVCC up to bus
wire(*apin("4"), 110, BUS_Y)
wire(*apin("18"), 112.54, BUS_Y)
# U1 GND down to GND symbol
wire(*apin("3"), 110, 150)
power("GND", 110, 150)

# J2 GND pin to GND symbol
J2_P2 = rpin(J2_PINS, 60, 158, 180, "2")          # (54.92, 155.46)
wire(*J2_P2, 54.92, 148)
power("GND", 54.92, 148)

# C1: VCC decoupling (top -> bus, bottom -> GND)
C1_T = rpin(C_PINS, 90, 60, 0, "1")               # (90, 56.19)
C1_B = rpin(C_PINS, 90, 60, 0, "2")               # (90, 63.81)
wire(*C1_T, 90, BUS_Y)
wire(*C1_B, 90, 70)
power("GND", 90, 70)
# C2: AVCC decoupling
C2_T = rpin(C_PINS, 125, 60, 0, "1")             # (125, 56.19)
C2_B = rpin(C_PINS, 125, 60, 0, "2")             # (125, 63.81)
wire(*C2_T, 125, BUS_Y)
wire(*C2_B, 125, 70)
power("GND", 125, 70)
# C3: AREF decoupling
C3_T = rpin(C_PINS, 80, 80, 0, "1")               # (80, 76.19)
C3_B = rpin(C_PINS, 80, 80, 0, "2")               # (80, 83.81)
AREF = apin("20")                                 # (94.76, 74.52)
poly(C3_T, (80, AREF[1]), AREF)
wire(*C3_B, 80, 90)
power("GND", 80, 90)

# R1: RESET pull-up to VCC
R1_T = rpin(R_PINS, 135, 80, 0, "1")             # (135, 76.19)
R1_B = rpin(R_PINS, 135, 80, 0, "2")             # (135, 83.81)
wire(*R1_T, 135, BUS_Y)
RST = apin("29")                                  # (125.24, 112.62)
poly(R1_B, (135, RST[1]), RST)

# J1 UART (rot 180). Use local net labels to avoid wire crossings.
J1_P1 = rpin(J3_PINS, 145, 118, 180, "1")        # (139.92, 120.54) RX
J1_P2 = rpin(J3_PINS, 145, 118, 180, "2")        # (139.92, 118)    TX
J1_P3 = rpin(J3_PINS, 145, 118, 180, "3")        # (139.92, 115.46) GND
PD0 = apin("30")                                  # (125.24, 117.7)
PD1 = apin("31")                                  # (125.24, 120.24)
wire(*PD0, 130, PD0[1])
label("RX", 130, PD0[1])
wire(*J1_P1, 145, J1_P1[1])
label("RX", 145, J1_P1[1])
wire(*PD1, 130, PD1[1])
label("TX", 130, PD1[1])
wire(*J1_P2, 145, J1_P2[1])
label("TX", 145, J1_P2[1])
wire(*J1_P3, 139.92, 108)                         # GND pin -> GND sym
power("GND", 139.92, 108)

# crystal pins: no-connect (internal oscillator)
noconnect(*apin("7"))
noconnect(*apin("8"))

# ---------------------------------------------------------------- footer
emit(f'  (sheet_instances (path "/" (page "1")))')
emit('  (embedded_fonts no)')
emit(")")

with open(OUT, "w") as f:
    f.write("\n".join(lines) + "\n")

# .kicad_pro (same as rev6/7)
pro = """{
  "board": {
    "3dviewports": [],
    "design_settings": {
      "defaults": {},
      "diff_pair_dimensions": [],
      "drc_exclusions": [],
      "meta": { "version": 3 },
      "rule_severities": {}
    },
    "layer_presets": [],
    "viewports": []
  },
  "boards": [],
  "cvpcb": {
    "equivalence_files": [],
    "libraries": []
  },
  "erc": {
    "erc_exclusions": [],
    "meta": { "version": 0 },
    "pin_map": []
  },
  "libraries": {
    "pinned_footprint_libs": [],
    "pinned_symbol_libs": []
  },
  "meta": {
    "filename": "%s.kicad_sch",
    "version": 1
  },
  "net_settings": {
    "classes": [
      {
        "bus_width": 12,
        "clearance": 0.2,
        "diff_pair_gap": 0.25,
        "diff_pair_via_gap": 0.25,
        "line_style": 0,
        "microvia_diameter": 0.3,
        "microvia_drill": 0.1,
        "name": "Default",
        "pcb_color": "rgba(0, 0, 0, 0.000)",
        "schematic_color": "rgba(0, 0, 0, 0.000)",
        "track_width": 0.25,
        "via_diameter": 0.8,
        "via_drill": 0.4,
        "wire_width": 0.15
      }
    ],
    "meta": { "version": 3 }
  },
  "pcbnew": {
    "last_paths": { "gencad": "", "idf": "", "netlist": "", "specctra_dsn": "", "step": "", "vrml": "" },
    "page_layout_descr_file": ""
  },
  "schematic": {
    "drawing": {
      "default_line_thickness": 6.0,
      "default_text_size": 50.0,
      "field_names": [],
      "intersheets_ref": { "prefix": "", "suffix": "" },
      "junction_size_choice": 3,
      "label_size_ratio": 0.375,
      "pin_symbol_size": 25.0,
      "text_offset_ratio": 0.15
    },
    "legacy_lib_dir": "",
    "legacy_lib_list": [],
    "meta": { "version": 1 },
    "net_format_name": "",
    "page_layout_descr_file": "",
    "plot_directory": "",
    "sheets": [
      [
        "%s",
        "Root"
      ]
    ],
    "symbol_libs": []
  },
  "sheets": [
    [
      "%s",
      "Root"
    ]
  ],
  "text_variables": {}
}
""" % (PROJ, ROOT_UUID, ROOT_UUID)
with open(PRO_OUT, "w") as f:
    f.write(pro)

print(f"wrote {OUT} ({os.path.getsize(OUT)} bytes)")
print(f"wrote {PRO_OUT}")
SCH_PATH = OUT
