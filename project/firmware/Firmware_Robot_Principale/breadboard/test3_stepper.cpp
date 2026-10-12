// TEST 3 C++: Stepper actuators on slot 2
// RESULTS : N/A

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
  
  stepperMotorsEnable(true);

}

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
