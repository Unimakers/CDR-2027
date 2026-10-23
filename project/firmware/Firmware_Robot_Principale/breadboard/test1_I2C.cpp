// TEST 1 C++: serial + I2C scan
// RESULTS : OK
//
// Lists every I2C address that answers. Expected on the main board:
//   0x40 PCA9685 U3, 0x41 PCA9685 U11 (JP10 soldered), PCF8574 ICM1 (PCF_ADDR in board_pins.h),
//   and 0x70: the "all call" address answered by both PCA9685 at the same time (only until
//   a firmware configures them: the Adafruit library turns it off, a power cycle turns it on).
// A missing address = chip not powered (3V3), not soldered, or wrong solder jumpers.
// Nothing at all = check SDA/SCL and the pull-ups R43/R44.
// To run it: copy this file over src/main.cpp.


#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// An empty transmission (address only, no data) is acknowledged only by a chip that
// has this address: endTransmission() then returns 0.
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
  Serial.println("[TEST] Breadboard test 1: serial + I2C scan");
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  scanI2C();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.printf("[TEST] alive %lu ms\n", millis());
  delay(1000);
}
