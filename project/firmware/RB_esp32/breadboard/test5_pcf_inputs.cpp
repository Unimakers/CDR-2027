// TEST 5 C++: body PCF8574 (driver ~EN / MS1 / MS2) and head inputs
// RESULTS : N/A
//
// No motor needed, driver boards plugged on NP1/NP2, 24V optional.
// - Toggles ~EN of both driver boards every 2 s and prints the PCF byte read back.
//   With a multimeter, check the ~EN pin of each TMC2209 (pin 16): ~3.3V when disabled,
//   ~0V when enabled. The PCF only has a weak pull-up (~100 uA) for the HIGH level:
//   if "disabled" reads well below 3.3V, the driver module pulls EN down and the motors
//   would never be disabled -> add a pull-up resistor on EN.
// - Prints the tirette, the team switch and the 4 head switches (J1..J4) every 200 ms.
// To run it: copy this file over src/main.cpp.

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "steppers.h"
#include "inputs.h"

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("[TEST] Breadboard test 5: PCF8574 drivers + head inputs");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  steppersBegin(); // writes MS1/MS2 from robot_config.h, drivers disabled
  inputsBegin();
}

void loop() {
  static unsigned long lastToggle = 0;
  static bool enabled = false;

  if (millis() - lastToggle >= 2000) {
    lastToggle = millis();
    enabled = !enabled;
    stepperMotorsEnable(enabled);
    // Read the PCF pins back (quasi-bidirectional: an output reads its real level)
    Wire.requestFrom((uint8_t)PCF_DRIVERS_ADDR, (uint8_t)1);
    if (Wire.available()) {
      uint8_t pins = Wire.read();
      Serial.printf("[TEST] driver PCF pins = 0x%02X (board0 EN=%d, board1 EN=%d)\n", pins,
                    (pins >> DRIVER_EN_PCF_BIT[0]) & 1, (pins >> DRIVER_EN_PCF_BIT[1]) & 1);
    }
  }

  Serial.printf("[TEST] tirette=%d team=%d switches=%d%d%d%d\n",
                tiretteIsInserted(), teamSwitchIsOn(),
                switchIsClosed(1), switchIsClosed(2), switchIsClosed(3), switchIsClosed(4));
  delay(200);
}
