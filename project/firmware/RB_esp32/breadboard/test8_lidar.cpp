// Current main = copy of breadboard/test8_lidar.cpp (LiDAR started by the tirette).
// To run another test, copy its file from breadboard/ over this one.
//
// TEST 8 C++: LiDAR LD06 + tirette
// RESULTS : OK (2026-11-24: 10 scans/s, ~456 points/scan, 3590 deg/s, 0 CRC error.
//           First try = 0 byte received: bad contact on the lidar connector, replugged)
//
// Wiring: LD06 on head J5 (data on GPIO43, see board_pins.h), tirette on head J6 (GPIO12),
// head board on NP3. No motor, no 24V needed.
// Sequence:
//   1. boot: if the tirette is not in place, insert it
//   2. pull the tirette -> the scans are streamed in teleplot format (https://teleplot.fr/)
//   3. put the tirette back -> streaming stops, pull it again to restart
// The LED blinks fast while streaming, slowly while waiting for the tirette.
// To run it: copy this file over src/main.cpp.

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "inputs.h"
#include "ld06.h"

#define TIRETTE_DEBOUNCE_MS 50

LD06 ld06(Serial1);

static bool streaming = false;

static void blinkBuiltinLed(uint32_t periodMs) {
#ifdef LED_BUILTIN
    static bool ledState = false;
    static uint32_t ref = 0;
    if (millis() - ref > periodMs) {
        ref = millis();
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState);
    }
#endif
}

// Tirette state, only changes after TIRETTE_DEBOUNCE_MS of stable reading
static bool tiretteDebounced() {
    static bool stable = tiretteIsInserted();
    static bool last = stable;
    static uint32_t changedAt = 0;
    bool now = tiretteIsInserted();
    if (now != last) {
        last = now;
        changedAt = millis();
    } else if (now != stable && millis() - changedAt >= TIRETTE_DEBOUNCE_MS) {
        stable = now;
    }
    return stable;
}

void setup() {
    Serial.begin(115200);
    unsigned long t0 = millis();
    while (!Serial && millis() - t0 < 5000) delay(10);
    delay(500);
    Serial.println("\n[TEST] Test 8: LiDAR LD06 + tirette");

#ifdef LED_BUILTIN
    pinMode(LED_BUILTIN, OUTPUT);
#endif

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    if (!inputsBegin()) {
        Serial.println("[TEST] head PCF8574 not answering (switches only, the tirette still works)");
    }

    // Do NOT call ld06.init(): it opens Serial1 on the default pins, wrong on this board.
    // Big RX buffer: printing a scan to USB takes several ms, the 256-byte default
    // would overflow meanwhile (CRC errors, lost packets)
    Serial1.setRxBufferSize(4096);

    // PWM input of the LD06 held LOW = internal speed control (~10 turns/s)
    pinMode(LD06_PWM_PIN, OUTPUT);
    digitalWrite(LD06_PWM_PIN, LOW);

    // Raw level probe, before the UART takes the pins: sample each pin as a plain input
    // for 200 ms and count the level changes. A healthy LD06 stream gives ~60% high and
    // thousands of changes (polling misses most edges at 230400 baud, but not all).
    //   0% high, 0 changes   -> line seen LOW all the time: level too weak or pulled down
    //   100% high, 0 changes -> idle high, no data reaches this pin
    //   ~60% high, many changes -> the ESP sees the data: the problem is the UART setup
    for (int pin : {LD06_RX_PIN, LD06_TX_PIN}) {
        pinMode(pin, INPUT);
        delay(5);
        uint32_t samples = 0, highs = 0, changes = 0;
        int last = digitalRead(pin);
        uint32_t tRaw = millis();
        while (millis() - tRaw < 200) {
            int v = digitalRead(pin);
            samples++;
            highs += v;
            if (v != last) { changes++; last = v; }
        }
        Serial.printf("[TEST] raw GPIO%d: %lu%% high, %lu changes in 200 ms (%lu samples)\n", pin,
                      (unsigned long)(100UL * highs / samples), (unsigned long)changes, (unsigned long)samples);
    }

    // Probe: listen 1 s on GPIO43, then on GPIO44 (in case TX/RX are swapped on the cable).
    // A running LD06 sends ~17 kB/s (375 packets of 47 bytes). Switching the pins can
    // produce a glitch byte or two, so flush after begin() and require a real amount of data.
    const int PROBE_MIN_BYTES = 1000;
    int rxPin = LD06_RX_PIN, txPin = LD06_TX_PIN;
    bool dataFound = false;
    for (int attempt = 0; attempt < 2; attempt++) {
        Serial1.begin(LD06_BAUDRATE, SERIAL_8N1, rxPin, txPin);
        delay(20);
        while (Serial1.available()) Serial1.read();
        int n = 0;
        uint32_t tProbe = millis();
        while (millis() - tProbe < 1000) {
            while (Serial1.available()) { Serial1.read(); n++; }
            delay(1);
        }
        Serial.printf("[TEST] probe: RX on GPIO%d -> %d bytes in 1 s\n", rxPin, n);
        if (n >= PROBE_MIN_BYTES) { dataFound = true; break; }
        Serial1.end();
        if (attempt == 0) { rxPin = LD06_TX_PIN; txPin = LD06_RX_PIN; }
        else { rxPin = LD06_RX_PIN; txPin = LD06_TX_PIN; Serial1.begin(LD06_BAUDRATE, SERIAL_8N1, rxPin, txPin); }
    }
    if (!dataFound) {
        Serial.println("[TEST] no data on GPIO43 nor GPIO44: does the lidar spin? check J5 (5V, GND, TX)");
    } else if (rxPin != LD06_RX_PIN) {
        Serial.println("[TEST] data found on GPIO44: TX/RX swapped, fix LD06_RX_PIN/LD06_TX_PIN in board_pins.h");
    }

    if (!tiretteIsInserted()) {
        Serial.println("[TEST] insert the tirette...");
        while (!tiretteDebounced()) {
            blinkBuiltinLed(500);
            delay(1);
        }
    }
    Serial.println("[TEST] tirette in place, pull it to start the lidar stream");
}

void loop() {
    // Read the UART on every pass, even when not streaming: the 256-byte buffer
    // overflows in ~11 ms and the first scan after the pull would be corrupted
    static uint32_t bytesRx = 0, scans = 0, lastPoints = 0;
    bytesRx += Serial1.available();
    bool newScan = ld06.readScan();
    if (newScan) {
        scans++;
        lastPoints = ld06.getNbPointsInScan();
    }

    // Diagnostic every second (a plain line = shown in the teleplot console):
    //   (bytes/s is approximate: it reads ~12500 when the lidar really sends ~17 kB/s)
    //   bytes/s ~0      -> nothing on GPIO43: wiring of J5, LD06 power, TX/RX swapped
    //   bytes/s >10000 but scans/s 0 -> data received but not decoded (crc errors?)
    //   scans/s ~10, points ~450    -> lidar OK
    static uint32_t lastDiag = 0;
    if (millis() - lastDiag >= 1000) {
        lastDiag = millis();
        Serial.printf("[DIAG] tirette=%s stream=%s bytes/s=%lu scans/s=%lu points=%lu speed=%u deg/s crcFail=%u\n",
                      tiretteDebounced() ? "in" : "out", streaming ? "on" : "off",
                      (unsigned long)bytesRx, (unsigned long)scans, (unsigned long)lastPoints,
                      ld06.getSpeed(), ld06.getChecksumFailCount());
        bytesRx = 0;
        scans = 0;
    }

    bool inserted = tiretteDebounced();
    if (streaming && inserted) {
        streaming = false;
        Serial.println("[TEST] tirette inserted -> stream stopped, pull it to restart");
    } else if (!streaming && !inserted) {
        streaming = true;
        Serial.println("[TEST] tirette pulled -> lidar stream started");
    }

    blinkBuiltinLed(streaming ? 100 : 500);
    if (streaming && newScan) {
        ld06.printScanTeleplot(Serial);
    }
}
