# Minimal Arduino (ATmega328P-AU) — KiCad schematic

Minimal all-SMD Arduino-compatible board. ATmega328P-AU (TQFP-32),
internal 8 MHz oscillator (no crystal).

## Circuit
- **U1** ATmega328P-AU — official KiCad `MCU_Microchip_ATmega:ATmega48PV-10A` symbol,
  Value set to ATmega328P-AU, footprint `Package_QFP:TQFP-32_7x7mm_P0.8mm`
- **R1** 10 kΩ 0805 — RESET pull-up to VCC
- **C1** 100 nF 0805 — VCC decoupling
- **C2** 100 nF 0805 — AVCC decoupling
- **C3** 100 nF 0805 — AREF decoupling
- **J1** 3-pin 2.54 mm SMD header — pin1 RX (PD0), pin2 TX (PD1), pin3 GND
- **J2** 2-pin 2.54 mm SMD header — pin1 +5V in, pin2 GND
- PB6/PB7: no-connect (no crystal)

All symbols are official KiCad library symbols embedded in the file
(sanitized to the v9 s-expr subset).

## Fuses (internal 8 MHz)
- LF `0xE2`, HF `0xDA`, EF `0x05`

## Files
- `generate_schematic.py` — generates `arduino-minimal-rev6.kicad_sch`
- `extract_symbols.py` — pulls official symbols from a local kicad-symbols checkout
- `validate.py` — sexp balance, connectivity, geometry, rotation-aware checks
- `erc_check.py` — standalone ERC: parses the s-expr, rebuilds nets from
  wires/pins/junctions/labels/power symbols, checks pin-type conflicts,
  unconnected pins, dangling wires, missing footprints
  (`python3 erc_check.py arduino-minimal-rev6.kicad_sch [--detail] [--pins]`)
- `arduino-minimal-rev6.kicad_sch` / `.kicad_pro` — deliverables (KiCad 9)

## ERC status (2026-10-08, rev6)
0 errors. All power pins reach their nets (VCC x2, AVCC tied to VCC, GND x3),
decoupling in place, RX->PD0, TX->PD1, RESET pull-up OK. Remaining warnings are
the unused GPIOs left unconnected, as intended for a minimal board.
