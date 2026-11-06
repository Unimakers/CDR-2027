// Current main = copy of breadboard/test0_find_pcf.cpp (I2C diagnosis, find the PCF8574).
// To run another test, copy its file from breadboard/ over this one.
//
// TEST 0 C++: find the PCF8574 - I2C bus diagnosis from scratch
// RESULTS : N/A
//
// Standalone: only Arduino + Wire, nothing else from the project.
//
// IMPORTANT, body v1 power: the 3V3 of the board (both PCF8574, I2C pull-ups R5/R6, head
// board) comes from the TSR1-2433 regulator U1, fed by the 24V. The 3V3 pin of the XIAO is
// NOT connected to it. With the USB cable only, the PCF are not powered and the bus has no
// pull-up: nothing answers. -> 24V (battery or supply) must be ON for this test.
//
// Steps:
//   1. Reads SDA (GPIO5) and SCL (GPIO6) with no pull-up at all: both must read 1, pulled up
//      by R5/R6 to the board 3V3. 0 = board 3V3 missing (no 24V, LED of the 3V3 off?)
//      or a line shorted to GND.
//   2. Same reading with the ESP32 internal pull-ups: 0 here = line shorted to GND.
//   3. Probes 0x20..0x27 (PCF8574) and 0x38..0x3F (PCF8574A) one by one and prints the
//      Wire error code: 0 = found, 2 = no answer (NACK), 5 = timeout (bus stuck).
//   4. Full scan at 100 kHz, then at 10 kHz (long wires / weak pull-ups).
//   5. Full scan with SDA/SCL swapped, in case the wiring is crossed.
// Expected: 0x20 (body PCF) and 0x21 (head PCF, JP1 soldered).
// To run it: copy this file over src/main.cpp.

#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 5 // D4, net "SDA"
#define SCL_PIN 6 // D5, net "SCL"

static void readLines(const char* label, int mode) {
    pinMode(SDA_PIN, mode);
    pinMode(SCL_PIN, mode);
    delay(5);
    int sda = digitalRead(SDA_PIN);
    int scl = digitalRead(SCL_PIN);
    Serial.printf("[LINES] %-22s SDA=%d SCL=%d %s\n", label, sda, scl,
                  (sda && scl) ? "OK" : "<-- a line is LOW");
}

static int scan(const char* label) {
    Serial.printf("[SCAN] %s...\n", label);
    int found = 0;
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t err = Wire.endTransmission();
        if (err == 0) {
            Serial.printf("[SCAN]   device at 0x%02X\n", address);
            found++;
        } else if (err == 5) {
            Serial.printf("[SCAN]   timeout at 0x%02X: bus stuck (SDA or SCL held LOW?)\n", address);
            break;
        }
    }
    Serial.printf("[SCAN] %s: %d device(s)\n", label, found);
    return found;
}

static void probePcfAddresses() {
    Serial.println("[PROBE] PCF8574 0x20..0x27 / PCF8574A 0x38..0x3F (0=found 2=NACK 5=timeout)");
    const uint8_t bases[2] = {0x20, 0x38};
    for (uint8_t base : bases) {
        Serial.print("[PROBE] ");
        for (uint8_t a = base; a < base + 8; a++) {
            Wire.beginTransmission(a);
            uint8_t err = Wire.endTransmission();
            Serial.printf("0x%02X:%d%s  ", a, err, err == 0 ? "*" : "");
        }
        Serial.println();
    }
}

void setup() {
    Serial.begin(115200);
    unsigned long t0 = millis();
    while (!Serial && millis() - t0 < 5000) delay(10); // USB CDC: wait for the monitor
    delay(500);
    Serial.println("\n[TEST] Test 0: find the PCF8574");
    Serial.println("[TEST] reminder: the board 3V3 comes from the 24V, USB alone is not enough");

    // 1-2. Line levels before the I2C driver takes the pins
    readLines("no pull-up (R5/R6):", INPUT);
    readLines("ESP32 pull-ups:", INPUT_PULLUP);

    // 3-4. Normal wiring
    if (!Wire.begin(SDA_PIN, SCL_PIN, 100000)) Serial.println("[TEST] Wire.begin failed");
    probePcfAddresses();
    scan("scan 100 kHz");
    Wire.setClock(10000);
    scan("scan 10 kHz");
    Wire.end();

    // 5. Swapped lines
    Wire.begin(SCL_PIN, SDA_PIN, 100000);
    scan("scan SDA/SCL SWAPPED");
    Wire.end();

    Wire.begin(SDA_PIN, SCL_PIN, 100000);
    Serial.println("[TEST] done, normal wiring scan repeated every 2 s (plug the 24V to see the PCF appear)");
}

void loop() {
    delay(2000);
    scan("rescan");
}
