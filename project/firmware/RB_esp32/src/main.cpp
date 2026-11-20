// Current main = copy of breadboard/test7_square.cpp (drive in a square).
// To run another test, copy its file from breadboard/ over this one.
//
// TEST 7 C++: drive in a square, forever (WHEEL_LEFT_SLOT / WHEEL_RIGHT_SLOT)
// RESULTS : N/A
//
// Wiring: same as test 6 (left wheel NP1 U2 = slot 1, right wheel NP2 U2 = slot 3), 24V ON,
// both PCF8574 answering (0x20 body, 0x21 head).
// Sequence: forward SQUARE_SIDE_MM, turn 90 deg on the spot, x4, pause, repeat.
// Wheel directions come from "inverted" in robot_config.h (validated with test 6):
// a positive move = this wheel drives the robot forward.
//
// Tuning:
//   - side too short/long  -> WHEEL_DIAMETER_MM
//   - turns over/under 90  -> WHEEL_BASE_MM (distance between the two wheel contact points,
//                             MEASURE IT on the robot, the value below is a guess)
//   - turns the wrong way  -> swap the sign of TURN_DIRECTION
//
// Serial commands (115200): p = pause/resume, d = disable motors (free wheels), e = enable.
// To run it: copy this file over src/main.cpp.

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "steppers.h"

#define WHEEL_DIAMETER_MM 79.0f // 2026 robot wheels
#define WHEEL_BASE_MM 300.0f    // TODO: measure (contact point to contact point)
#define SQUARE_SIDE_MM 500.0f
#define TURN_DIRECTION 1        // 1 = left turns (counter-clockwise square), -1 = right turns

static FastAccelStepper* leftWheel = nullptr;
static FastAccelStepper* rightWheel = nullptr;
static bool paused = false;
static bool motorsOn = false;

// microsteps per mm of wheel travel (same steps/rev on both wheels)
static float stepsPerMm() {
    return stepperSlotStepsPerRev(WHEEL_LEFT_SLOT) / (PI * WHEEL_DIAMETER_MM);
}

static long distanceToSteps(float mm) {
    return lroundf(mm * stepsPerMm());
}

// On-the-spot rotation: each wheel travels an arc of radius WHEEL_BASE_MM / 2
static long angleToSteps(float deg) {
    return distanceToSteps(deg * PI / 180.0f * (WHEEL_BASE_MM / 2.0f));
}

static void setMotors(bool on) {
    stepperMotorsEnable(on);
    motorsOn = on;
}

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

// Relative move of both wheels, started together, then waits for both to stop
static void moveWheels(const char* label, long leftSteps, long rightSteps) {
    while (paused || !motorsOn) { handleSerial(); delay(10); }
    Serial.printf("[MOVE] %s: L %+ld  R %+ld usteps\n", label, leftSteps, rightSteps);
    if (leftWheel) leftWheel->move(leftSteps);
    if (rightWheel) rightWheel->move(rightSteps);
    while ((leftWheel && leftWheel->isRunning()) || (rightWheel && rightWheel->isRunning())) {
        handleSerial();
        delay(1);
    }
    delay(300); // let the robot settle before the next move
}

static void forward(float mm) {
    long s = distanceToSteps(mm);
    moveWheels("forward", s, s);
}

// deg > 0 = counter-clockwise (left), deg < 0 = clockwise (right)
static void turn(float deg) {
    long s = angleToSteps(deg);
    moveWheels("turn", -s, s);
}

void setup() {
    Serial.begin(115200);
    unsigned long t0 = millis();
    while (!Serial && millis() - t0 < 5000) delay(10);
    delay(500);
    Serial.println("\n[TEST] Test 7: square");
    Serial.println("[TEST] commands: p = pause/resume, d = disable motors, e = enable");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    steppersBegin();
    leftWheel = connectStepperSlot(WHEEL_LEFT_SLOT);
    rightWheel = connectStepperSlot(WHEEL_RIGHT_SLOT);
    if (!leftWheel || !rightWheel) {
        Serial.println("[TEST] ERROR: a wheel is not usable, check robot_config.h");
    }
    Serial.printf("[TEST] %.2f usteps/mm, side %.0f mm = %ld usteps, 90 deg = %ld usteps per wheel\n",
                  stepsPerMm(), SQUARE_SIDE_MM, distanceToSteps(SQUARE_SIDE_MM), angleToSteps(90));

    setMotors(true);
    Serial.println("[TEST] starting in 3 s...");
    delay(3000);
}

void loop() {
    for (int i = 0; i < 4; i++) {
        forward(SQUARE_SIDE_MM);
        turn(TURN_DIRECTION * 90.0f);
    }
    Serial.printf("[TEST] square done (uptime %lu ms)\n", millis());
    delay(2000);
}
