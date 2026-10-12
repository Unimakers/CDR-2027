#include <Arduino.h>
#include "config.h"
#include "match.h"

static unsigned long matchStartMs = 0;
static bool started = false;
static bool endLogged = false;

void matchStart() {
    matchStartMs = millis();
    started = true;
    Serial.println("[MATCH] Match started");
}

bool matchHasStarted() {
    return started;
}

bool matchHasEnded() {
    if (!started) return false;
    bool ended = (millis() - matchStartMs) >= MATCH_DURATION * 1000UL;
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
        delay(1);
    }
    return !matchHasEnded();
}
