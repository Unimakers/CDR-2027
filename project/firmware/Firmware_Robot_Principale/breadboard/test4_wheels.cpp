// TEST 3 C++: Stepper actuators on slot 2
// RESULTS : N/A

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "actuators.h"
#include "steppers.h"
#include "match.h"

FastAccelStepper* leftWheel = nullptr;
FastAccelStepper* rightWheel = nullptr;

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
    
    steppersBegin();

    leftWheel = connectStepperSlot(WHEEL_LEFT_SLOT);
    rightWheel = connectStepperSlot(WHEEL_RIGHT_SLOT);

    stepperMotorsEnable(true);

}

void loop() {
    long oneTurn = stepperSlotStepsPerRev(WHEEL_LEFT_SLOT); // both wheels have the same steps/rev

    Serial.printf("[TEST] moving wheels forward 1 turn (%ld steps)\n", oneTurn);
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
