# Minimal Arduino (ATmega328P-AU) — KiCad schematic

Minimal all-SMD Arduino-compatible board. ATmega328P-AU (TQFP-32),
internal 8 MHz oscillator (no crystal).

## Revisions
- **rev8 (current, 2026-10-08)** — Regenerated with correct KiCad
  coordinate transform. Opens in KiCad 10, ERC clean (see below),
  netlist verified. **Use this one.**
- rev7 — Fixed file-format bugs (unit names, lib_symbols headers,
  `(unlocked)`), loads in KiCad, but wires were routed to wrong
  pin positions (see "coordinate bug" below). Superseded by rev8.
- rev6 and earlier — do not open in KiCad (format bugs). Superseded.

## Circuit
- **U1** ATmega328P-AU — official KiCad `MCU_Microchip_ATmega:ATmega48PV-10A` symbol,
  Value set to ATmega328P-AU, footprint `Package_QFP:TQFP-32_7x7mm_P0.8mm`
- **R1** 10 kΩ 0805 — RESET pull-up to VCC
- **C1** 100 nF 0805 — VCC decoupling
- **C2** 100 nF 0805 — AVCC decoupling (AVCC tied to VCC per datasheet)
- **C3** 100 nF 0805 — AREF decoupling
- **J1** 3-pin 2.54 mm SMD header — pin1 RX (PD0), pin2 TX (PD1), pin3 GND
- **J2** 2-pin 2.54 mm SMD header — pin1 +5V in, pin2 GND
- PB6/PB7: no-connect (no crystal)

All symbols are official KiCad library symbols embedded in the file.

## Fuses (internal 8 MHz)
- LF `0xE2`, HF `0xDA`, EF `0x05`

## The coordinate bug (rev6/7)
KiCad's symbol libraries are authored Y-UP, but the schematic sheet is
Y-DOWN. On placement KiCad converts: pin (px, py) on a symbol at (X, Y)
rotated t° clockwise lands at
`(X + px·cos t − py·sin t, Y − px·sin t − py·cos t)`.
The rev6/7 generator wired to `(X+px, Y+py)` (no conversion), so in KiCad
every wire missed its pin (89 ERC violations, all "pin not connected").
`generate_schematic.py` now uses `kicad_pin()` with the correct transform;
rev8 was re-routed for it (VCC bus along the top, GND below).

## ERC status (2026-10-08, rev8, KiCad 10.0.6 native ERC)
Netlist verified correct: VCC (U1.4/U1.6/AVCC/C1/C2/R1/J2.1),
GND (U1.3/U1.5/U1.21/C1/C2/C3/J1.3/J2.2), RX (PD0↔J1.1),
TX (PD1↔J1.2), RESET pull-up, AREF decoupling.
Remaining ERC items are benign/expected:
- 20× unconnected pins — unused GPIOs (PD2-PD7, PB0-PB5, PC0-PC5,
  ADC6/ADC7), as intended for a minimal board
- `power_pin_not_driven` on VCC/GND — normal without PWR_FLAG symbols
- `endpoint_off_grid` — library pins not on 1.27mm grid; they still connect
- `lib_symbol_issues` / `footprint_link_issues` — headless environment
  lacks library table config; symbols/footprints are embedded/assigned

## Files
- `generate_schematic.py` — generates `arduino-minimal-rev8.kicad_sch`
- `extract_symbols.py` — pulls official symbols from a local kicad-symbols checkout
  (unit sub-symbols must keep short names, e.g. `R_0_1` not `Device:R_0_1`;
  no version/generator lines inside `lib_symbols`)
- `validate.py` — sexp balance, connectivity, geometry, rotation-aware checks
- `erc_check.py` — standalone ERC (now uses the correct Y-up→Y-down transform)
- `arduino-minimal-rev8.kicad_sch` / `.kicad_pro` — deliverables (KiCad 9/10)
- `rev8.pdf` / `rev8-preview.png` — rendered schematic
