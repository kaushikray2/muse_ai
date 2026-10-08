/*
 * Battery Capacity Calibration Tester -- Arduino firmware
 * ========================================================
 * Concept: single cell under test -> 12 ohm discharge resistor ->
 * MOSFET cutoff switch -> GND. Arduino measures cell voltage on A0,
 * drives the MOSFET gate on D7, and talks to the PC GUI over USB serial.
 *
 * Wiring (match the concept drawing):
 *   Battery+  -> one end of the 12 ohm load resistor
 *   Battery+  -> A0 (cell voltage sense). Direct wire is fine for a
 *                single Li-ion/LiFePO4 cell (<=4.2 V) with the default
 *                5 V ADC reference. For higher voltages use a divider
 *                and set DIVIDER_RATIO below.
 *   Battery-  -> Arduino GND (common ground, required)
 *   Other end of 12 ohm resistor -> NMOS drain; NMOS source -> GND
 *   (low-side switch). D7 drives the gate directly -- use a logic-level
 *   NMOS (Vgs(th) well under 5 V).
 *   USB       -> PC (115200 baud)
 *
 * Safety notes:
 *   - 12 ohm at 4.2 V dissipates ~1.5 W. Use a 5 W (or bigger) resistor
 *     and keep it away from anything meltable; it gets hot.
 *   - The load is OFF at boot and on STOP. The firmware also enforces a
 *     maximum test time as a failsafe.
 *
 * Serial protocol (115200 8N1, lines terminated with \n):
 *   PC -> Arduino:
 *     PING                              ->  PONG
 *     START <cutoff_mV> <interval_ms> <max_minutes>
 *                                           ->  ACK START | ERR ...
 *     STOP                              ->  STOPPED
 *   Arduino -> PC:
 *     READY                             (sent once at boot)
 *     DATA <voltage_mV> <elapsed_s> <discharged_mAh>
 *     DONE <capacity_mAh> CUTOFF|TIMEOUT
 */

const uint8_t VBAT_PIN = A0;     // cell voltage sense
const uint8_t LOAD_PIN = 7;      // MOSFET gate drive

// Low-side NMOS: gate HIGH turns the load ON. Flip to LOW only if your
// driver stage inverts the signal.
const bool LOAD_ON_LEVEL = HIGH;

// Voltage divider on A0: VBAT -- R1 -- A0 -- R2 -- GND
//   DIVIDER_RATIO = R2 / (R1 + R2).  Direct wire: 1.0
//   Example 10k/10k divider for up to ~8.4 V: 0.5
const float DIVIDER_RATIO = 1.0;

// ADC oversampling for noise reduction
const uint8_t ADC_SAMPLES = 16;

// Must match the physical discharge resistor (ohms)
const float LOAD_OHMS = 12.0;

enum RunState { IDLE, RUNNING };
RunState runState = IDLE;

unsigned long tStartMs   = 0;
unsigned long tLastMs    = 0;
unsigned long intervalMs = 1000;
float         cutoffV    = 3.0;
unsigned long maxDurMs   = 12UL * 3600UL * 1000UL;
float         dischargedMah = 0.0;

String inBuf;

void setLoad(bool on) {
  digitalWrite(LOAD_PIN, on ? LOAD_ON_LEVEL : !LOAD_ON_LEVEL);
}

float readBatteryV() {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < ADC_SAMPLES; i++) {
    sum += analogRead(VBAT_PIN);
    delay(2);
  }
  float v = (sum / (float)ADC_SAMPLES) * (5.0 / 1023.0);
  return v / DIVIDER_RATIO;
}

void finishRun(bool reachedCutoff) {
  setLoad(false);
  runState = IDLE;
  Serial.print("DONE ");
  Serial.print(dischargedMah, 2);
  Serial.print(' ');
  Serial.println(reachedCutoff ? "CUTOFF" : "TIMEOUT");
}

void execCommand(String cmd) {
  cmd.trim();
  if (cmd == "PING") {
    Serial.println("PONG");
  }
  else if (cmd == "STOP") {
    if (runState == RUNNING) {
      setLoad(false);
      runState = IDLE;
      Serial.println("STOPPED");
    } else {
      Serial.println("ACK IDLE");
    }
  }
  else if (cmd.startsWith("START")) {
    // START <cutoff_mV> <interval_ms> <max_minutes>
    int p1 = cmd.indexOf(' ');
    int p2 = cmd.indexOf(' ', p1 + 1);
    int p3 = cmd.indexOf(' ', p2 + 1);
    if (p1 < 0 || p2 < 0 || p3 < 0) {
      Serial.println("ERR bad START args (want: START <cutoff_mV> <interval_ms> <max_minutes>)");
      return;
    }
    cutoffV    = cmd.substring(p1 + 1, p2).toFloat() / 1000.0;
    intervalMs = (unsigned long)cmd.substring(p2 + 1, p3).toInt();
    maxDurMs   = (unsigned long)cmd.substring(p3 + 1).toInt() * 60000UL;
    if (intervalMs < 200) intervalMs = 200;
    if (cutoffV <= 0.5 || cutoffV > 5.0) {
      Serial.println("ERR cutoff out of range (0.5..5.0 V)");
      return;
    }
    dischargedMah = 0.0;
    tStartMs = millis();
    tLastMs  = tStartMs;
    runState = RUNNING;
    setLoad(true);
    Serial.println("ACK START");
  }
  else {
    Serial.println("ERR unknown command");
  }
}

void handleSerial() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (inBuf.length() > 0) { execCommand(inBuf); inBuf = ""; }
    } else if (inBuf.length() < 80) {
      inBuf += c;
    }
  }
}

void setup() {
  pinMode(LOAD_PIN, OUTPUT);
  setLoad(false);              // load OFF at boot -- safe state
  Serial.begin(115200);
  Serial.println("READY");
}

void loop() {
  handleSerial();

  if (runState == RUNNING) {
    unsigned long now = millis();
    if (now - tLastMs >= intervalMs) {
      float dtHours = (now - tLastMs) / 3600000.0;
      float v       = readBatteryV();
      float iAmps   = v / LOAD_OHMS;
      dischargedMah += iAmps * dtHours * 1000.0;   // mAh
      tLastMs = now;

      Serial.print("DATA ");
      Serial.print((int)round(v * 1000.0));        // mV
      Serial.print(' ');
      Serial.print((now - tStartMs) / 1000);       // elapsed s
      Serial.print(' ');
      Serial.println(dischargedMah, 2);           // mAh

      if (v <= cutoffV)      finishRun(true);
      else if (now - tStartMs >= maxDurMs) finishRun(false);
    }
  }
}
