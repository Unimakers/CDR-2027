// TEST 1 C++: serial + I2C scan
// RESULTS : OK
//
// Lists every I2C address that answers. Expected on the body v1 board (XIAO ESP32-S3 Plus):
//   0x20 PCF8574T ICM1 of the body (driver ~EN / MS1 / MS2, no jumper),
//   0x21 PCF8574T ICM1 of the head (JP1 soldered on the head, otherwise it collides on 0x20),
//   0x40 / 0x41 the external PCA9685 modules (if plugged), and 0x70: the "all call" address
//   answered by every PCA9685 (only until a firmware configures them).
// 0x38..0x3F instead of 0x20..0x27 = the chip is a PCF8574A: change PCF_*_ADDR in board_pins.h.
// A missing address = chip not powered (3V3), not soldered, or wrong solder jumpers.
// Nothing at all = check SDA/SCL (GPIO5/GPIO6) and the pull-ups R5/R6.
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
