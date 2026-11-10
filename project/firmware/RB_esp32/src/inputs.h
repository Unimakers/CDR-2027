// Inputs of the head board: tirette and team switch on XIAO GPIOs, 4 switches on the
// head PCF8574 (J1..J4). Every contact closes to GND.
//
// Setup order in main: Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN); inputsBegin();
//
//   while (tiretteIsInserted()) delay(1);   // wait for the cord to be pulled
//   matchStart();

#pragma once

#include <Arduino.h>

// Configures the GPIO inputs and starts the head PCF8574.
// Returns false if the head PCF does not answer (switchIsClosed() then returns false).
bool inputsBegin();

bool tiretteIsInserted(); // true while the starting cord is in place (contact closed)
bool teamSwitchIsOn();    // team switch contact closed

// Switch n (1..SWITCH_COUNT, as J1..J4 on the head). Reads the PCF over I2C on each call.
bool switchIsClosed(int n);
