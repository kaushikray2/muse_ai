# Phase 2: 4-Layer Layout Notes — Tracker RA0E2

**Date:** 2026-10-09  
**Board:** tracher_2026.kicad_pcb (RA0E2 migrated)

## Completed

### Board Setup
- **Layers:** 4-layer (F.Cu / In1.Cu / In2.Cu / B.Cu)
- **Stackup (JLCPCB 4L 1.6mm):**
  - L1 Top: signal, 1oz
  - Prepreg: ~0.2mm
  - L2 In1.Cu: GND plane, 1oz
  - Core: ~1.0mm
  - L3 In2.Cu: +3.3V plane, 1oz
  - Prepreg: ~0.2mm
  - L4 Bottom: signal, 1oz
- **Design rules:** 0.15mm min trace/space, 0.3mm min drill, 0.15mm annular ring
- **Board outline:** 110 x 55mm rectangular, (90.3, 46.9) to (200.3, 101.9)
- **Mounting holes:** 2x MH1/MH2, 3.2mm NPTH drill, 6mm pad, at (95.3, 51.9) and (195.3, 96.9)
- **Copper zones:** GND on In1.Cu, +3.3V on In2.Cu (full board pours)

### Netlist Sync (from RA0E2 schematic)
- U1 pads updated to RA0E2 pinout (47 pins)
- J1 rewired for SWD (1:RST, 2:NC, 3:GND, 4:+3.3V, 5:SWDIO, 6:SWCLK)
- Added C10 (1µF VCL), C11 (100nF VCC)
- Removed C3, R1, R2, R5 (deleted in schematic)
- Fixed 16x 0.2mm thermal vias → 0.3mm drill (JLC standard)

### Placement
Verified good:
- U1 (MCU) central
- IC1 (LoRa) + J2 (SMA) at left edge
- U4 (GNSS) + J6 (SMA) upper area, ≥20mm from J2
- J5 (USB-C) at right edge
- U5 (magnetometer) at right side, away from PA
- C10/C11 within 4mm of U1

## Remaining: Signal Routing

**Status:** Board is placed and plane-ready. Signal routing not yet done.

**Nets needing routing:** 63 nets (198 unconnected items)
- GND (112 pads) → handled by L2 plane (needs via stitching)
- +3.3V (42 pads) → handled by L3 plane (needs via stitching)
- V_BAT+ (12 pads) → needs manual routing (power section, right side)
- RF: Net-(IC1-ANT), /GNSS/GNSS_RF_IN → needs short direct routing
- ~60 signal nets → need routing

**Why not autorouted:** Copperline cannot parse net names with parentheses (e.g., "Net-(D4-K)"). Manual Python routing of 63 nets was not feasible in the available time.

**Recommended next steps:**
1. Open in KiCad GUI
2. Route V_BAT+ (wide, 0.5mm+) in power section
3. Route RF nets (short, direct, no vias)
4. Route SPI/UART/I2C/SWD
5. Route remaining signals
6. Add GND stitching vias
7. Fill zones, run DRC

## DRC Status
- 282 violations (mostly unconnected, library warnings, silkscreen)
- 0 shorts
- Board is structurally sound

## Files
- tracher_2026.kicad_pcb — 4-layer board, placed, planes defined
- tracher_2026.kicad_sch — top sheet (unchanged)
- micro.kicad_sch — RA0E2 migrated (Phase 1)
- power.kicad_sch, gps.kicad_sch — unchanged
- MIGRATION-NOTES.md — Phase 1 details
