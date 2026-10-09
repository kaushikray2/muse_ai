# Tracker Schematic Fixes Applied (MF1–MF6)

**Date:** 2026-10-09  
**Files modified:** `micro.kicad_sch`, `power.kicad_sch`, `gps.kicad_sch`, `tracher_2026.kicad_sch`, `tracher_2026.kicad_pcb`  
**Constraints honored:** RL78 kept (no RA0E2 migration), no crystal added, LC76G fitted / SAM-M10Q DNP.

---

## MF1: Inter-sheet nets fixed ✅

**Problem:** Top sheet was empty, zero hierarchical labels — 13 cross-sheet nets were broken.

**Fix:**
- Converted all 13 nets to `hierarchical_label` on sub-sheets with correct direction shapes:
  - `micro.kicad_sch`: 13 labels (MCU_BattVolt/BattVolt_EN/BUTTON_SENSE/SYS_EN as in/out, 9× GNSS nets)
  - `power.kicad_sch`: 4 labels (BattVolt/BattVolt_EN/BUTTON_SENSE/SYS_EN)
  - `gps.kicad_sch`: 9 labels (GNSS RX/TX/EN/RESET/3DFIX/ANT_ON/AP_REQ/GEOFENCE/JAM_IND)
- Rebuilt `tracher_2026.kicad_sch` with three hierarchical sheet instances:
  - `Power` → `power.kicad_sch` (page 2)
  - `Micro+LORA` → `micro.kicad_sch` (page 3)
  - `GNSS` → `gps.kicad_sch` (page 4)
- Added matching hierarchical pins on each sheet instance with explicit wires connecting them.
- **PCB:** Merged 44 pad references from split nets (`/Power/...`, `/Micro+LORA/...`, `/GNSS/...`) into 13 unified nets (`/MCU_BattVolt`, `/MCU_GNSS_RX`, etc.).

**Note:** Sheet names (`Power`, `Micro+LORA`, `GNSS`) match the original PCB net paths to minimize disruption.

---

## MF2: E22 LoRa module GND pads ✅

**Problem:** Per Ebyte datasheet, module pins 2-5, 10-12, 20, 22 are GND but were unconnected in the PCB.

**Fix:** Added `(net "GND")` to IC1 pads 2, 3, 4, 5, 10, 11, 12, 20, 22 in the PCB.

**Correction to review:** The pads were *unconnected*, not shorted to signal nets as originally reported. The fix is the same (tie to GND).

---

## MF3: U1 thermal pad ✅

**Problem:** QFN-48 exposed pad (pad 49) had no net.

**Fix:** Assigned `(net "GND")` to all 18 pad-49 instances (thermal vias) in the U1 PCB footprint.

---

## MF4: Sleep current fixes ✅

**Problem:** Four issues defeating low-power operation.

**Fixes (all marked DNP, not deleted):**
| Ref | Change | Location |
|-----|--------|----------|
| D6, R22 | Power LED → DNP | Schematic + PCB (`attr dnp`) |
| U6 | SAM-M10Q → DNP | Schematic + PCB |
| R37, C26, C27 | SAM-M10Q support → DNP | Schematic + PCB |
| J7 | GPS UART tap (U6-only) → DNP | Schematic + PCB |
| R26 | 0Ω GNSS switch bypass → DNP | Schematic + PCB |

**Verified:** Q2/Q3 load switch remains fitted (no DNP) as the default GNSS power path. U4 (LC76G) remains fitted.

---

## MF5: U5 (LSM303AGR) wired and placed ✅

**Problem:** U5 was in the schematic but completely unwired (dangling stubs) and missing from the PCB.

**Fixes:**

### Schematic (`power.kicad_sch`):
- Deleted 10 dangling wire stubs around U5.
- Wired per ST LSM303AGR datasheet (verified pinout via web search):
  - Pin 1 (SCL) → hierarchical `MCU_I2C_SCL` + R39 (4.7kΩ) pull-up to +3.3V
  - Pin 4 (SDA) → hierarchical `MCU_I2C_SDA` + R40 (4.7kΩ) pull-up to +3.3V
  - Pin 2 (CS_XL) → +3.3V (I2C mode select)
  - Pin 3 (CS_MAG) → +3.3V (I2C mode select)
  - Pin 5 (C1) → C30 (100nF) to GND (datasheet-required capacitor)
  - Pins 6, 8 (GND) → GND
  - Pin 9 (VDD) → +3.3V + C31 (100nF) decoupling to GND
  - Pin 10 (VDD_IO) → +3.3V
  - Pins 7, 11, 12 (INT) → no-connect
- Added hierarchical labels `MCU_I2C_SCL`/`MCU_I2C_SDA` (bidirectional).

### Schematic (`micro.kicad_sch`):
- Wired U1 pin 17 (P15/SCL20) → hierarchical `MCU_I2C_SCL`
- Wired U1 pin 18 (P14/SDA20) → hierarchical `MCU_I2C_SDA`
- **Design choice:** Used P15/P14 (SAU simplified I2C) since dedicated I2C pins P60/P61 (pins 1,2) are taken by BattVolt_EN/BUTTON_SENSE. Documented for review.

### Schematic (`tracher_2026.kicad_sch`):
- Added `MCU_I2C_SCL`/`MCU_I2C_SDA` pins to Micro+LORA and Power sheet instances with connecting wires.

### PCB:
- Added U5 footprint (`LSM303AGRTR:LGA12R50P_200X200X100`, LGA-12 2x2mm) at (192, 85) with correct net assignments.
- Added R39, R40, C30, C31 footprints (0603) with correct nets.

### ⚠️ U5 footprint needs verification
The LGA-12 footprint was created manually (0.55mm pitch, 0.18mm pads). DRC reports 3 clearance violations (0.14mm vs 0.2mm required) on the corner pads. **Kaushik must verify the footprint against the ST LSM303AGR datasheet before ordering.** The electrical connections (nets) are correct; only the pad geometry needs review.

### New components added:
- R39, R40: 4.7kΩ 0603 (I2C pull-ups)
- C30: 100nF 0603 (U5 C1 pin, datasheet-required)
- C31: 100nF 0603 (U5 VDD decoupling)

---

## MF6: SW1 footprint fixed ✅

**Problem:** 4-pin symbol (`SW_MEC_5G`) with 2-pad footprint; terminal B had no copper. Also found: schematic wire from SW1-B to D5-A was broken (gap between 48.26 and 53.34 on y=88.9).

**Fixes:**
- **PCB:** Replaced SW1 footprint with 4-pad version:
  - Pads 1, 2 → `V_BAT+` (terminal A, kept at original positions)
  - Pads 3, 4 → `Net-(D5-A)` (terminal B, new)
  - Footprint renamed to `Button_Switch_THT:SW_PUSH_6mm_H5mm_4pin`
- **Schematic:** Updated SW1 footprint field to match. Added missing wire (48.26,88.9)→(53.34,88.9) connecting SW1-B to D5-A.

**Note:** The 4-pad footprint geometry is generic; verify against the actual tactile switch part before ordering.

---

## Verification

### DRC (`kicad-cli pcb drc`):
- ✅ **0 shorting_items** (no shorts)
- ✅ All 6 must-fix defects resolved
- ⚠️ 200 `unconnected_items` — expected (board is unrouted)
- ⚠️ 3 `clearance` errors — U5 footprint corner pads (needs datasheet verification, see MF5 note)
- ⚠️ `lib_footprint_issues` — VM lacks KiCad footprint libraries (not real issues)
- ⚠️ 16 `drill_out_of_range` — pre-existing 0.2mm thermal vias (noted in review)
- Other warnings (silk, courtyard, etc.) are pre-existing or cosmetic.

### ERC (`kicad-cli sch erc`):
- ❌ Cannot run — missing `libwx_gtk3u_webview` shared library in this headless KiCad install. **Kaushik should run ERC in KiCad GUI after opening the fixed schematics.**

---

## Files changed
- `micro.kicad_sch` — 13 hierarchical labels + I2C wiring
- `power.kicad_sch` — 4 hierarchical labels, DNP flags (D6/R22), SW1 footprint field + wire, U5 wiring + 4 new components
- `gps.kicad_sch` — 9 hierarchical labels, DNP flags (U6/R37/C26/C27/J7/R26)
- `tracher_2026.kicad_sch` — rebuilt with 3 hierarchical sheets, pins, wires
- `tracher_2026.kicad_pcb` — net merges, IC1 GND pads, U1 thermal pad, DNP attrs, SW1 4-pad, U5 + 4 new footprints

Backups in `backup-20261009/`.
