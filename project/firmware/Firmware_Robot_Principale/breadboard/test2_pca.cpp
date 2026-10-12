// TEST 2 C++: PCA9685 servo driver
// RESULTS : N/A
// TEST 2 bis C++ : matchWait() is used to wait for the servo to move, instead of delay() which would block the PCA9685 driver task. The servo moves to 0, 90 and 180 degrees with a 1 second pause between each move. The serial output shows the alive message every 3 seconds.
// RESULTS : N/A

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "actuators.h"
#include "match.h"

ServoActuator testServo("testServo", 1); // connector 1, pulse range 0.5..2.5 ms

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
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  actuatorsBegin();
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
