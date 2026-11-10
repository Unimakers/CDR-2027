// PCF8574 driver, see pcf8574.h. Protocol: a write = address + 1 data byte,
// a read = address + 1 data byte returned by the chip. No registers.

#include "pcf8574.h"

Pcf8574::Pcf8574(uint8_t address, TwoWire& wire)
    : _address(address), _wire(&wire), _state(0xFF), _ready(false) {}

bool Pcf8574::begin(uint8_t initialState) {
    _wire->beginTransmission(_address);
    if (_wire->endTransmission() != 0) {
        Serial.printf("[PCF] ERROR: no PCF8574 answering at 0x%02X\n", _address);
        _ready = false;
        return false;
    }
    _ready = true;
    if (!write(initialState)) return false;
    Serial.printf("[PCF] PCF8574 ready at 0x%02X\n", _address);
    return true;
}

bool Pcf8574::write(uint8_t value) {
    if (!_ready) return false;
    _wire->beginTransmission(_address);
    _wire->write(value);
    if (_wire->endTransmission() != 0) {
        Serial.printf("[PCF] ERROR: write to 0x%02X failed\n", _address);
        return false;
    }
    _state = value;
    return true;
}

bool Pcf8574::writeBit(uint8_t bit, bool high) {
    if (bit > 7) return false;
    uint8_t mask = (uint8_t)(1u << bit);
    return writeMasked(mask, high ? mask : 0);
}

bool Pcf8574::writeMasked(uint8_t mask, uint8_t value) {
    return write((uint8_t)((_state & ~mask) | (value & mask)));
}

bool Pcf8574::read(uint8_t* value) {
    if (!_ready) return false;
    if (_wire->requestFrom(_address, (uint8_t)1) != 1) {
        Serial.printf("[PCF] ERROR: read from 0x%02X failed\n", _address);
        return false;
    }
    *value = (uint8_t)_wire->read();
    return true;
}
