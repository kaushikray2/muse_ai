# RA0E2 48-Pin Migration Notes

**Date:** 2026-10-09  
**Source:** `tracher_2026/` (RL78 R7F100GGN, fixed 2026-10-09)  
**Target:** `tracher_2026_ra0e2/` (Renesas RA0E2, 48-pin LQFP 7x7mm — hand-solderable for prototypes)  
**File changed:** `micro.kicad_sch` (U1 symbol + rewiring). `power.kicad_sch`, `gps.kicad_sch`, `tracher_2026.kicad_sch` unchanged (hierarchical label names preserved).

## Pin Mapping Table (RA0E2 R7FA0E2094CFM)

| Pin | RA0E2 Name | Function | Net | Notes |
|-----|-----------|----------|-----|-------|
| 1 | P400/SCLA1 | I2C SCL out | MCU_I2C_SCL (hier) | To U5 LSM303AGR |
| 2 | P401/SDAA1 | I2C SDA out | MCU_I2C_SDA (hier) | To U5 LSM303AGR |
| 3 | VCL | LDO output | 1µF (C10) to GND | Datasheet-required |
| 4 | P215 | GPIO out | MCU_BattVolt_EN (hier) | |
| 5 | P214 | GPIO out | MCU_SYS_EN (hier) | |
| 6 | VSS | GND | GND | |
| 7 | P213 | GPIO out | MCU_GNSS_EN (hier) | |
| 8 | P212 | GPIO out | MCU_GNSS_RESET (hier) | |
| 9 | VCC | +3.3V | +3.3V + 100nF (C11) | |
| 10 | P409 | GPIO in | MCU_GNSS_3DFIX (hier) | |
| 11 | P408 | GPIO in | MCU_GNSS_ANT_ON (hier) | |
| 12 | P407 | GPIO in | MCU_GNSS_AP_REQ (hier) | |
| 13 | P915 | GPIO in | MCU_GNSS_GEOFENCE (hier) | |
| 14 | P914 | GPIO in | MCU_GNSS_JAM_IND (hier) | |
| 15 | P913 | — | NC (no_connect) | Free |
| 16 | P208 | GPIO in | MCU_LORA_BUSY | LoRa BUSY |
| 17 | P207/IRQ2 | GPIO in | MCU_LORA_DIO1 | LoRa DIO1 (IRQ2) |
| 18 | P206 | — | NC (no_connect) | Free |
| 19 | RES | Reset in | MCU_RST (R4 10k + C4 10nF) | Also to J1.5 nRESET |
| 20 | P201 | GPIO out | MCU_LORA_TXEN | LoRa TXEN |
| 21 | P200 | GPIO in | MCU_BUTTON_SENSE (hier) | Button sense |
| 22 | P302 | — | NC (no_connect) | Free |
| 23 | P301 | — | NC (no_connect) | Free |
| 24 | P300/SWCLK | SWD clock | J1.2 SWCLK | Wired to TC2030 |
| 25 | P108/SWDIO | SWD data | J1.1 SWDIO | Wired to TC2030 |
| 26 | P109/TXD2 | UART TX | MCU_GNSS_TX (hier) | To GNSS module RX |
| 27 | P110/RXD2 | UART RX | MCU_GNSS_RX (hier) | From GNSS module TX |
| 28 | P111 | — | NC (no_connect) | Free |
| 29 | P112 | GPIO out | MCU_LORA_RST | LoRa NRST |
| 30 | P106 | — | NC (no_connect) | Free |
| 31 | P105 | — | NC (no_connect) | Free |
| 32 | P104 | — | NC (no_connect) | Free |
| 33 | P103 | GPIO out | MCU_LED | LED via R3/D1 |
| 34 | P102/SCK00 | SPI SCK | MCU_LORA_SCK | LoRa SCK |
| 35 | P101/SO00 | SPI MISO | MCU_LORA_MISO | LoRa MISO |
| 36 | P100/SI00 | SPI MOSI | MCU_LORA_MOSI | LoRa MOSI |
| 37 | P500 | — | NC (no_connect) | Free |
| 38 | P015 | GPIO out | MCU_LORA_NSS | LoRa NSS |
| 39 | P014 | GPIO out | MCU_LORA_RXEN | LoRa RXEN |
| 40 | P013 | — | NC (no_connect) | Free |
| 41 | P012 | — | NC (no_connect) | Free |
| 42 | P009 | — | NC (no_connect) | Free |
| 43 | P008 | — | NC (no_connect) | Free |
| 44 | P011/VREFL0 | GND | GND | ADC ref low |
| 45 | P010/AN000 | ADC in | MCU_BattVolt (hier) | Battery voltage |
| 46 | P002 | — | NC (no_connect) | Free |
| 47 | P001 | — | NC (no_connect) | Free |
| 48 | P000 | — | NC (no_connect) | Free |

## Schematic Changes

### New symbol `Custom:R7FA0E2094CFM`
- Created in `micro.kicad_sch` lib_symbols, replacing `Custom:R7F100GGN`.
- 48 pins, same body size/position as old symbol (U1 stays at 180.34, 97.79).
- Pin electrical types: VCC/VSS/VREFL0=power_in, VCL=power_out, RES=input, rest=bidirectional.
- **Note:** KiCad symbol y-axis is inverted vs schematic; pin y-values in the definition are negated to land on the intended schematic coordinates. Verified via netlist export.

### Removed
- **C3** (1µF on REGC) — RA0E2 has no REGC pin. Its GND-side wire stub also removed.
- **R5** (1kΩ TOOL0 pull-up) — RL78 debug, unwired. Removed.
- **R1/R2** — not present in schematic (already gone).
- **J1 old wiring** — RL78 UART debug (MCU_TOOL, MCU_D_TX, MCU_D_RX) removed. J1 rewired for SWD (see below).
- **TP3, TP4** — testpoints on removed nets (MCU_TOOL, old J1.4).
- **I2C/SPI short** — the MF5 fix had accidentally shorted MCU_I2C_SCL→MCU_LORA_MISO and MCU_I2C_SDA→MCU_LORA_MOSI (hier labels placed on SPI wires). Removed the shorted hier labels/wires; I2C now on dedicated P400/P401.
- **Dangling stubs** — several wire stubs around U1 left side (old VDD/REGC/RESET/TOOL0) removed.
- **Stale pin UUIDs** — U1 instance `(pin "N" (uuid ...))` entries from RL78 removed (they caused netlist pin mismatches).

### Added
- **C10** (1µF/16V, 0603) — VCL (pin 3) to GND, datasheet-required.
- **C11** (100nF/16V, 0603) — VCC (pin 9) decoupling, close to pin.
- **GND** on VSS (pin 6) and VREFL0 (pin 44).
- **+3.3V** on VCC (pin 9).
- **RES** (pin 19) wired to MCU_RST net (R4 10k pull-up + C4 10nF + J1.5).
- **SWD**: P25→J1.1 (SWDIO), P24→J1.2 (SWCLK) via drawn wires.
- **J1 (TC2030) SWD pinout**: 1:SWDIO, 2:SWCLK, 3:GND, 4:+3.3V, 5:nRESET (MCU_RST), 6:SWO (no_connect).
- **no_connect** flags on all 16 unused GPIOs (pins 15,18,22,23,28,30,31,32,37,40,41,42,43,46,47,48).
- **I2C hier labels** moved to P400/P401 (pins 1,2).

### Kept
- RESET circuit R4 (10k) + C4 (10nF) — unchanged, now on RES pin 19.
- LED circuit D1 + R3 (220Ω) — unchanged, now on P103 (pin 33).
- All LoRa nets (NSS/MOSI/MISO/SCK/RXEN/TXEN/BUSY/DIO1/NRST) — rewired to new pins, same net names.
- All hierarchical labels/names unchanged — top sheet (`tracher_2026.kicad_sch`) needs no changes.
- U1 footprint set to `Package_QFP:LQFP-48_7x7mm_P0.5mm` (2026-10-09: changed from QFN to LQFP for hand-soldering prototypes; same die/pinout, no thermal pad).

## Pre-existing Issues Found (in RL78 source, now fixed)
The RL78 "fixed" schematic had incomplete MCU wiring:
- VDD, VSS, RESET, REGC pins were floating/dangling (wires didn't reach pins).
- VSS was on a mislabeled net "MCU_TOOL" (also shorted to J1.1).
- 12 hierarchical labels (BattVolt, SYS_EN, GNSS_*, BUTTON_SENSE, etc.) were dangling (wires stopped at x=213.36 without touching pins).
- I2C was shorted to SPI (MF5 wiring error).
All fixed in this migration by rewiring every functional net to the correct RA0E2 pin.

## Verification
- **Netlist export** (`kicad-cli sch export netlist`): all 32 functional U1 pins on correct nets, 16 NC pins unconnected. ✅
- **ERC** (`kicad-cli sch erc` on micro.kicad_sch standalone):
  - 0 errors related to the migration.
  - Expected warnings (standalone mode): 15× hier-label `pin_not_connected`/`isolated_pin_label` (need parent sheet), 2× `power_pin_not_driven` (#PWR01/#PWR02, power comes from power sheet), 22× `endpoint_off_grid` (new components on 1.27mm grid), 45× `lib_symbol_issues` + 30× `footprint_link_issues` (VM lacks KiCad libraries).
- **Top sheet**: not modified (hier names unchanged). Note: `kicad-cli sch erc` fails to load the top sheet hierarchically ("Failed to load schematic") — pre-existing quirk, also happens on unmodified files. Kaushik should run ERC in KiCad GUI.

## Open Items for Kaushik / Phase 2
- Verify U1 footprint `Package_QFP:LQFP-48_7x7mm_P0.5mm` against RA0E2 48-pin LFQFP datasheet (e.g. R7FA0E2148CFP).
- New components C10, C11 need PCB footprints placed near U1 (Phase 2 layout).
- J1 TC2030 pinout changed to SWD — verify against Tag-Connect cable.
- Run ERC in KiCad GUI (hierarchical) to confirm zero errors.
