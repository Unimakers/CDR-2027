// TEST 6 C++: wheels only (WHEEL_LEFT_SLOT / WHEEL_RIGHT_SLOT), verbose debug
// RESULTS : N/A
//
// Wiring, as on the 2026 robot: left wheel on J2 (U2) of the driver board plugged on NP1
// (slot 1), right wheel on J2 (U2) of the driver board on NP2 (slot 3), 24V present.
// Head board optional (its PCF is on 0x21, JP1 soldered).
// Uses the wheel lines of STEPPER_SLOTS in robot_config.h (values of the 2026 firmware).
//
// Sequence, repeated forever:
//   1. left wheel alone: 1 turn forward, 1 turn back
//   2. right wheel alone: 1 turn forward, 1 turn back
//   3. both wheels: 1 turn forward, 1 turn back (straight line)
//   4. both wheels opposite: 1 turn each way (rotation on the spot)
// Positions are printed every 100 ms during each move.
// If a wheel turns the wrong way in step 1 or 2, set "inverted" to true on its line.
//
// Serial commands (115200, send one letter):
//   p = pause / resume the sequence (motors stay enabled while paused)
//   d = disable the motors (free wheels), e = enable them again
//
// Debug checklist:
//   [I2C] no 0x20         -> body PCF missing: ~EN cannot go LOW, nothing moves
//   "slot X: nullptr"     -> a -1 left in robot_config.h ([CONFIG] line says which)
//   positions change but the wheel does not turn -> 24V, motor wiring, or ~EN (see PCF readback)
//   wheel vibrates/whines without turning -> maxSpeedHz/accelerationHz2 too high, or coil pair swapped
// To run it: copy this file over src/main.cpp.

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "steppers.h"

static FastAccelStepper* leftWheel = nullptr;  // nullptr if the slot config is invalid
static FastAccelStepper* rightWheel = nullptr;
static bool paused = false;
static bool motorsOn = false;

// ---------------------------------------------------------------------------
// Debug helpers
// ---------------------------------------------------------------------------
static void scanI2C() {
    Serial.println("[I2C] Scanning bus...");
    int found = 0;
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            const char* what = "";
            if (address == PCF_DRIVERS_ADDR) what = " <- PCF8574 body (drivers)";
            else if (address == PCF_HEAD_ADDR) what = " <- PCF8574 head";
            else if (address == PCA_1_ADDR || address == PCA_2_ADDR) what = " <- PCA9685";
            else if (address == 0x70) what = " <- PCA9685 all-call";
            Serial.printf("[I2C]   device at 0x%02X%s\n", address, what);
            found++;
        }
    }
    Serial.printf("[I2C] %d device(s) found\n", found);
}

// Reads the body PCF and prints ~EN / MS1 / MS2 of both driver boards (one wheel on each)
static void printDriverPcf(const char* when) {
    Wire.requestFrom((uint8_t)PCF_DRIVERS_ADDR, (uint8_t)1);
    if (!Wire.available()) {
        Serial.printf("[PCF] (%s) body PCF 0x%02X does not answer\n", when, PCF_DRIVERS_ADDR);
        return;
    }
    uint8_t pins = Wire.read();
    Serial.printf("[PCF] (%s) pins=0x%02X\n", when, pins);
    for (int b = 0; b < DRIVER_BOARD_COUNT; b++) {
        int en = (pins >> DRIVER_EN_PCF_BIT[b]) & 1;
        int ms1 = (pins >> DRIVER_MS1_PCF_BIT[b]) & 1;
        int ms2 = (pins >> DRIVER_MS2_PCF_BIT[b]) & 1;
        Serial.printf("[PCF]   board %d (NP%d): ~EN=%d (%s) MS1=%d MS2=%d\n",
                      b, b + 1, en, en ? "disabled" : "ENABLED", ms1, ms2);
    }
}

static void printSlot(int slot, FastAccelStepper* s) {
    const StepperSlot& c = STEPPER_SLOTS[slot];
    Serial.printf("[CFG] slot %d: STEP GPIO%d DIR GPIO%d inverted=%d %d steps x 1/%d = %ld usteps/turn, "
                  "vmax=%d Hz acc=%d Hz/s -> %s\n",
                  slot, c.stepPin, c.dirPin, c.inverted, c.stepsPerRev, c.microsteps,
                  stepperSlotStepsPerRev(slot), c.maxSpeedHz, c.accelerationHz2,
                  s ? "connected" : "nullptr (not usable)");
}

static void setMotors(bool on) {
    stepperMotorsEnable(on);
    motorsOn = on;
    printDriverPcf(on ? "after enable" : "after disable");
}

// p / d / e over the serial monitor
static void handleSerial() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == 'p') {
            paused = !paused;
            Serial.println(paused ? "[TEST] PAUSED (send p to resume)" : "[TEST] resumed");
        } else if (c == 'd') {
            if (leftWheel) leftWheel->forceStop();
            if (rightWheel) rightWheel->forceStop();
            setMotors(false);
        } else if (c == 'e') {
            setMotors(true);
        }
    }
}

// Waits for both wheels to stop, printing the positions every 100 ms.
// Gives up after timeoutMs (a move that never ends = speed/acceleration problem).
static void waitWheels(const char* label, unsigned long timeoutMs = 10000) {
    unsigned long start = millis();
    unsigned long lastPrint = 0;
    while ((leftWheel && leftWheel->isRunning()) || (rightWheel && rightWheel->isRunning())) {
        handleSerial();
        if (millis() - lastPrint >= 100) {
            lastPrint = millis();
            Serial.printf("[MOVE] %-14s t=%5lu ms  L pos=%7ld %s  R pos=%7ld %s\n", label, millis() - start,
                          leftWheel ? (long)leftWheel->getCurrentPosition() : 0L,
                          leftWheel && leftWheel->isRunning() ? "run " : "stop",
                          rightWheel ? (long)rightWheel->getCurrentPosition() : 0L,
                          rightWheel && rightWheel->isRunning() ? "run " : "stop");
        }
        if (millis() - start > timeoutMs) {
            Serial.printf("[MOVE] %s: TIMEOUT after %lu ms, forcing stop\n", label, timeoutMs);
            if (leftWheel) leftWheel->forceStop();
            if (rightWheel) rightWheel->forceStop();
            break;
        }
        delay(1);
    }
    Serial.printf("[MOVE] %-14s done in %lu ms  L pos=%ld  R pos=%ld\n", label, millis() - start,
                  leftWheel ? (long)leftWheel->getCurrentPosition() : 0L,
                  rightWheel ? (long)rightWheel->getCurrentPosition() : 0L);
}

// Relative move of each wheel (in microsteps, 0 = that wheel does not move), then waits
static void moveWheels(const char* label, long leftSteps, long rightSteps) {
    while (paused) { handleSerial(); delay(10); }
    if (!motorsOn) {
        Serial.printf("[MOVE] %s skipped: motors disabled (send e)\n", label);
        delay(500);
        return;
    }
    Serial.printf("[MOVE] %s: L %+ld  R %+ld usteps\n", label, leftSteps, rightSteps);
    if (leftWheel && leftSteps != 0) {
        MoveResultCode r = leftWheel->move(leftSteps);
        if (r != MOVE_OK) Serial.printf("[MOVE] left move refused (code %d)\n", (int)r);
    }
    if (rightWheel && rightSteps != 0) {
        MoveResultCode r = rightWheel->move(rightSteps);
        if (r != MOVE_OK) Serial.printf("[MOVE] right move refused (code %d)\n", (int)r);
    }
    waitWheels(label);
    delay(500);
}

// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    // USB CDC: give the monitor up to 5 s to connect so the start-up logs are not lost
    unsigned long t0 = millis();
    while (!Serial && millis() - t0 < 5000) delay(10);
    delay(500);
    Serial.println();
    Serial.println("[TEST] Test 6: wheels only, debug");
    Serial.println("[TEST] commands: p = pause/resume, d = disable motors, e = enable");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    scanI2C();

    steppersBegin(); // PCF: MS1/MS2 from robot_config.h, ~EN disabled
    printDriverPcf("after steppersBegin");

    leftWheel = connectStepperSlot(WHEEL_LEFT_SLOT);
    rightWheel = connectStepperSlot(WHEEL_RIGHT_SLOT);
    printSlot(WHEEL_LEFT_SLOT, leftWheel);
    printSlot(WHEEL_RIGHT_SLOT, rightWheel);
    if (!leftWheel && !rightWheel) {
        Serial.println("[TEST] ERROR: no wheel usable, fix robot_config.h ([CONFIG] lines above)");
    }

    setMotors(true);
    Serial.println("[TEST] starting in 2 s...");
    delay(2000);
}

void loop() {
    static unsigned long cycle = 0;
    long leftTurn = stepperSlotStepsPerRev(WHEEL_LEFT_SLOT);
    long rightTurn = stepperSlotStepsPerRev(WHEEL_RIGHT_SLOT);

    Serial.printf("\n[TEST] ===== cycle %lu (uptime %lu ms) =====\n", ++cycle, millis());

    moveWheels("left fwd", leftTurn, 0);
    moveWheels("left back", -leftTurn, 0);

    moveWheels("right fwd", 0, rightTurn);
    moveWheels("right back", 0, -rightTurn);

    moveWheels("both fwd", leftTurn, rightTurn);
    moveWheels("both back", -leftTurn, -rightTurn);

    moveWheels("spin A", leftTurn, -rightTurn);
    moveWheels("spin B", -leftTurn, rightTurn);

    delay(1000);
}
