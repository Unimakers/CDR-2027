#include <Arduino.h>
#include <Wire.h>
#include "config.h"

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
