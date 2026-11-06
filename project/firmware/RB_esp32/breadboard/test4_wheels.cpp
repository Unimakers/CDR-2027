// TEST 4 C++: wheels (WHEEL_LEFT_SLOT / WHEEL_RIGHT_SLOT = slots 1 and 3)
// RESULTS : N/A
//
// Wiring: left wheel on J2 (U2) of the driver board on NP1 (slot 1), right wheel on J2 (U2)
// of the driver board on NP2 (slot 3), 24V present.
// Before uploading, fill the wheel lines of STEPPER_SLOTS in robot_config.h:
// with -1 values connectStepperSlot() returns nullptr and the wheel is skipped.
// Expected: both wheels one turn forward, pause, one turn backward, pause, forever.
// If one wheel turns the wrong way, set "inverted" to true on its line in robot_config.h.
// Uses FastAccelStepper directly (in microsteps), without StepperActuator.
// To run it: copy this file over src/main.cpp.

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "actuators.h"
#include "steppers.h"
#include "match.h"

FastAccelStepper* leftWheel = nullptr;  // nullptr if the slot config is invalid
FastAccelStepper* rightWheel = nullptr;

// move() returns immediately: wait until both motors have stopped
static void waitWheels() {
    while ((leftWheel && leftWheel->isRunning()) || (rightWheel && rightWheel->isRunning())) {
        delay(1);
    }
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay(2000);
    Serial.println("[TEST] Breadboard test 4: wheels");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN); // ~EN and MS1/MS2 are on the body PCF8574
    steppersBegin();

    leftWheel = connectStepperSlot(WHEEL_LEFT_SLOT);
    rightWheel = connectStepperSlot(WHEEL_RIGHT_SLOT);

    stepperMotorsEnable(true);

}

void loop() {
    long oneTurn = stepperSlotStepsPerRev(WHEEL_LEFT_SLOT); // both wheels have the same steps/rev

    Serial.printf("[TEST] moving wheels forward 1 turn (%ld steps)\n", oneTurn);
    // move() is relative: both wheels start together and run at the same time
    if (leftWheel) leftWheel->move(oneTurn);
    if (rightWheel) rightWheel->move(oneTurn);
    waitWheels();
    delay(1000);

    Serial.println("[TEST] Both wheels: one turn backward");
    if (leftWheel) leftWheel->move(-oneTurn);
    if (rightWheel) rightWheel->move(-oneTurn);
    waitWheels();
    delay(1000);
    
    Serial.printf("[TEST] alive %lu ms\n", millis());
    delay(1000);
}
