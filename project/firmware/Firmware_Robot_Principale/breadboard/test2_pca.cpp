// TEST 2 C++: PCA9685 servo driver
// RESULTS : N/A
// TEST 2 bis C++ : matchWait() is used instead of delay() to wait for the servo to move,
// so the wait stops as soon as the match is over. The servo moves to 0, 90 and 180 degrees
// with a 1 second pause between each move. The serial output shows the alive message every 3 seconds.
// RESULTS : N/A
//
// Wiring: one servo on connector "Servo1" (PCA9685 U3, output LED1), 5VP converter on.
// If the servo does not move: check the [ACT] lines at startup (PCA9685 found?) and the 5VP.
// To run it: copy this file over src/main.cpp.

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "actuators.h"
#include "match.h"

ServoActuator testServo("testServo", 1); // connector 1, pulse range 0.5..2.5 ms

// Same scan as test 1, not called here: add scanI2C() in setup() to debug the bus.
static void scanI2C() {
    Serial.println("[I2C] Scanning bus...");
    int found = 0;
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            Serial.printf("[I2C]   device at 0x%02X\n", address);
            found++;
        }
    }
    Serial.printf("[I2C] %d device(s) found\n", found);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(2000);
  Serial.println("[TEST] Breadboard test 2: pca9685");
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN); // I2C first: actuatorsBegin() talks to the PCA9685s
  actuatorsBegin();                     // starts the PCAs and testServo
}

void loop() {
  // put your main code here, to run repeatedly:
  testServo.setAngle(0);
  matchWait(1000);

  testServo.setAngle(90);
  matchWait(1000);

  testServo.setAngle(180);
  matchWait(1000);

  Serial.printf("[TEST] alive %lu ms\n", millis());
}
