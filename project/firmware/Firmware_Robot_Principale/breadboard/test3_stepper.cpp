// TEST 3 C++: Stepper actuators on slot 2
// RESULTS : N/A
//
// Wiring: one stepper motor on J8 (TMC2209 U8 = slot 2), 24V present.
// Before uploading, fill the slot 2 line of STEPPER_SLOTS in robot_config.h (stepsPerRev,
// microsteps matching JP4/JP5, maxSpeedHz, accelerationHz2): with -1 values the actuator
// refuses to start and every moveTo() is ignored with an [ACT] log.
// Expected: one motor turn forward, pause, one turn back, pause, forever.
// To run it: copy this file over src/main.cpp.

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "actuators.h"
#include "steppers.h"
#include "match.h"

StepperActuator testStepper("testStepper", 2, 360.0f); // slot 2, 1 motor turn = 360°

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(2000);
  Serial.println("[TEST] Breadboard test 3: stepper actuators on slot 2");
  
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  
  steppersBegin();
  
  actuatorsBegin(); // also starts the PCA9685: [ACT]/[CONFIG] lines are normal if none is wired
  
  stepperMotorsEnable(true); // ~EN LOW: the motors hold their position from now on

}

// moveTo() is blocking: each line returns once the motor has arrived.
// The match is never started in this test, so matchWait() behaves like delay().
void loop() {
    testStepper.moveTo(360.0f);
    Serial.printf("[TEST] moving to %1.0f\n", testStepper.getPosition());
    matchWait(500);

    testStepper.moveTo(0.0f);
    Serial.printf("[TEST] moving to %1.0f\n", testStepper.getPosition());
    matchWait(500);


    Serial.printf("[TEST] alive %lu ms\n", millis());
    matchWait(1000);
}
