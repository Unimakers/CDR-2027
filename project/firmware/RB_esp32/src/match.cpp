// Match timer, see match.h. Based on millis(): the unsigned subtraction
// "millis() - matchStartMs" stays correct when millis() wraps around (after ~49 days).

#include <Arduino.h>
#include "config.h"
#include "match.h"

static unsigned long matchStartMs = 0; // millis() value when matchStart() was called
static bool started = false;
static bool endLogged = false; // the end-of-match message is printed only once

void matchStart() {
    matchStartMs = millis();
    started = true;
    Serial.println("[MATCH] Match started");
}

bool matchHasStarted() {
    return started;
}

bool matchHasEnded() {
    if (!started) return false; // preparation time: nothing is blocked
    bool ended = (millis() - matchStartMs) >= MATCH_DURATION;
    if (ended && !endLogged) {
        endLogged = true;
        Serial.println("[MATCH] Match time is over");
    }
    return ended;
}

unsigned long matchElapsedMs() {
    return started ? millis() - matchStartMs : 0;
}

bool matchWait(unsigned long durationMs) {
    unsigned long start = millis();
    while (millis() - start < durationMs) {
        if (matchHasEnded()) return false;
        // delay(1) instead of a busy loop: lets the other FreeRTOS tasks (WiFi, idle
        // task feeding the watchdog...) run, and gives a 1 ms reaction time to the match end.
        delay(1);
    }
    return !matchHasEnded();
}
