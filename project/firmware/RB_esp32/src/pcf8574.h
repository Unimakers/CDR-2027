// Minimal PCF8574 driver (8-bit quasi-bidirectional I/O expander, I2C, 100 kHz).
//
// The chip has no direction register: writing 1 to a pin turns on a weak pull-up
// (~100 uA, the pin can then be read as an input or pulled low from outside),
// writing 0 drives it strongly LOW. At power-on every pin is 1.
// This class keeps a copy of the last written byte, so one bit can be changed without
// touching the others (inputs and unused pins must stay at 1).
//
// Wire must be started (Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN)) before begin().
// Not thread-safe: use it from the Arduino loop task only.

#pragma once

#include <Arduino.h>
#include <Wire.h>

class Pcf8574 {
public:
    Pcf8574(uint8_t address, TwoWire& wire = Wire);

    // Checks that the chip answers and writes initialState (default: all pins HIGH).
    // Returns false (with a log) if nothing answers at the address.
    bool begin(uint8_t initialState = 0xFF);
    bool isReady() const { return _ready; }

    bool write(uint8_t value);                // all 8 pins at once
    bool writeBit(uint8_t bit, bool high);    // one pin, the others keep their level
    // Several pins in a single I2C transaction: bits set in mask take the value of the
    // same bits in value.
    bool writeMasked(uint8_t mask, uint8_t value);

    // Reads the 8 pins. Returns false if the chip did not answer (value left unchanged).
    bool read(uint8_t* value);

    uint8_t lastWritten() const { return _state; }
    uint8_t getAddress() const { return _address; }

private:
    uint8_t _address;
    TwoWire* _wire;
    uint8_t _state;
    bool _ready;
};
