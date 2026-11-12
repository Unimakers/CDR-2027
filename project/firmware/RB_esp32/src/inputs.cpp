// Head board inputs, see inputs.h.

#include <Wire.h>
#include "config.h"
#include "inputs.h"
#include "pcf8574.h"

static Pcf8574 headPcf(PCF_HEAD_ADDR, Wire);

bool inputsBegin() {
    // No external pull-up on tir / TSW: the contacts only pull to GND
    pinMode(TIRETTE_PIN, INPUT_PULLUP);
    pinMode(TEAM_SWITCH_PIN, INPUT_PULLUP);
    // All pins HIGH = weak pull-ups on: every switch pin is an input
    return headPcf.begin(0xFF);
}

bool tiretteIsInserted() {
    return digitalRead(TIRETTE_PIN) == LOW;
}

bool teamSwitchIsOn() {
    return digitalRead(TEAM_SWITCH_PIN) == LOW;
}

bool switchIsClosed(int n) {
    if (n < 1 || n > SWITCH_COUNT) {
        Serial.printf("[CONFIG] Switch %d does not exist (1..%d)\n", n, SWITCH_COUNT);
        return false;
    }
    uint8_t pins;
    if (!headPcf.read(&pins)) return false;
    return (pins & (1u << SWITCH_PCF_BIT[n - 1])) == 0; // closed contact pulls the pin to 0
}
