// Stepper slots, see steppers.h. One FastAccelStepper engine drives every slot;
// each slot is connected at most once and kept in slotSteppers[].
// STEP/DIR are XIAO GPIOs; ~EN and MS1/MS2 of each driver board are on the body PCF8574.

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "steppers.h"
#include "pcf8574.h"

static FastAccelStepperEngine engine;
// Motor already connected on each slot (nullptr = not connected yet)
static FastAccelStepper* slotSteppers[STEPPER_SLOT_COUNT] = {nullptr};
static Pcf8574 driversPcf(PCF_DRIVERS_ADDR, Wire);

static bool isSupportedMicrostep(int microsteps) {
    // TMC2209 standalone mode, see the MS1/MS2 table in board_pins.h
    return microsteps == 2 || microsteps == 4 || microsteps == 8 || microsteps == 16;
}

// MS1/MS2 levels for a microstep value (TMC2209 standalone table)
static void microstepLevels(int microsteps, bool* ms1, bool* ms2) {
    switch (microsteps) {
        case 2:  *ms1 = true;  *ms2 = false; break;
        case 4:  *ms1 = false; *ms2 = true;  break;
        case 8:  *ms1 = false; *ms2 = false; break;
        default: *ms1 = true;  *ms2 = true;  break; // 16
    }
}

// Both TMC2209 of a driver board share MS1/MS2: their slots need the same microsteps.
// Returns the board value, or -1 if no slot of the board has a valid value
// (the board then stays at 1/16, the power-on state of the PCF).
static int boardMicrosteps(int board) {
    int value = -1;
    for (int slot = 0; slot < STEPPER_SLOT_COUNT; slot++) {
        if (STEPPER_SLOT_BOARD[slot] != board) continue;
        int ms = STEPPER_SLOTS[slot].microsteps;
        if (!isSupportedMicrostep(ms)) continue;
        if (value < 0) {
            value = ms;
        } else if (value != ms) {
            Serial.printf("[CONFIG] Driver board %d: slots share MS1/MS2 but have different "
                          "microsteps (%d and %d), using %d\n", board, value, ms, value);
        }
    }
    return value;
}

void steppersBegin() {
    engine.init();

    // ~EN and MS1/MS2: the PCF starts with every pin HIGH (drivers disabled, 1/16).
    // Write the microsteps of each board with ~EN still inactive.
    uint8_t state = 0xFF;
    for (int board = 0; board < DRIVER_BOARD_COUNT; board++) {
        bool enLevel = DRIVERS_EN_ACTIVE_LOW; // disabled
        if (enLevel) state |= (1u << DRIVER_EN_PCF_BIT[board]);
        else state &= ~(1u << DRIVER_EN_PCF_BIT[board]);

        int ms = boardMicrosteps(board);
        if (ms < 0) ms = 16;
        bool ms1, ms2;
        microstepLevels(ms, &ms1, &ms2);
        if (ms1) state |= (1u << DRIVER_MS1_PCF_BIT[board]);
        else state &= ~(1u << DRIVER_MS1_PCF_BIT[board]);
        if (ms2) state |= (1u << DRIVER_MS2_PCF_BIT[board]);
        else state &= ~(1u << DRIVER_MS2_PCF_BIT[board]);
        Serial.printf("[STEPPER] Driver board %d: 1/%d microstep\n", board, ms);
    }
    if (!driversPcf.begin(state)) {
        Serial.println("[STEPPER] ERROR: driver PCF8574 missing, the motors cannot be enabled");
    }
    Serial.println("[STEPPER] Engine initialized, motors disabled");
}

// Checks the STEPPER_SLOTS line of the slot (robot_config.h). Prints every problem found,
// not only the first one, so a single upload shows everything that is left to fill in.
static bool checkSlotConfig(int slot) {
    if (slot < 0 || slot >= STEPPER_SLOT_COUNT) {
        Serial.printf("[CONFIG] Stepper slot %d does not exist (0..%d)\n", slot, STEPPER_SLOT_COUNT - 1);
        return false;
    }
    const StepperSlot& cfg = STEPPER_SLOTS[slot];
    bool ok = true;
    if (cfg.stepPin < 0 || cfg.dirPin < 0) {
        Serial.printf("[CONFIG] Slot %d: STEP/DIR pin not set in board_pins.h\n", slot);
        ok = false;
    }
    if (cfg.stepsPerRev <= 0) {
        Serial.printf("[CONFIG] Slot %d: stepsPerRev not set\n", slot);
        ok = false;
    }
    if (!isSupportedMicrostep(cfg.microsteps)) {
        Serial.printf("[CONFIG] Slot %d: microsteps must be 2, 4, 8 or 16 (got %d)\n", slot, cfg.microsteps);
        ok = false;
    } else if (cfg.microsteps != boardMicrosteps(STEPPER_SLOT_BOARD[slot])) {
        // the board got another slot's value: the distances of this slot would be wrong
        Serial.printf("[CONFIG] Slot %d: microsteps differ from the other slot of its driver board\n", slot);
        ok = false;
    }
    if (cfg.maxSpeedHz <= 0 || cfg.accelerationHz2 <= 0) {
        Serial.printf("[CONFIG] Slot %d: maxSpeedHz / accelerationHz2 not set\n", slot);
        ok = false;
    }
    return ok;
}

FastAccelStepper* connectStepperSlot(int slot) {
    if (!checkSlotConfig(slot)) return nullptr;
    if (slotSteppers[slot]) return slotSteppers[slot]; // already connected: same motor object

    const StepperSlot& cfg = STEPPER_SLOTS[slot];
    // Fails if the GPIO cannot generate pulses or if every pulse generator is already taken
    FastAccelStepper* stepper = engine.stepperConnectToPin(cfg.stepPin);
    if (!stepper) {
        Serial.printf("[STEPPER] ERROR: slot %d, GPIO%d cannot be used as a STEP pin\n", slot, cfg.stepPin);
        return nullptr;
    }
    // Second argument = DIR level for positive moves: inverting it reverses the motor
    // without touching the wiring.
    stepper->setDirectionPin(cfg.dirPin, !cfg.inverted);
    // ~EN is shared per driver board and lives on the PCF: handled by stepperMotorsEnable()
    stepper->setAutoEnable(false);
    stepper->setSpeedInHz(cfg.maxSpeedHz);
    stepper->setAcceleration(cfg.accelerationHz2);

    slotSteppers[slot] = stepper;
    Serial.printf("[STEPPER] Slot %d connected (STEP GPIO%d, DIR GPIO%d, driver board %d)\n",
                  slot, cfg.stepPin, cfg.dirPin, STEPPER_SLOT_BOARD[slot]);
    return stepper;
}

bool stepperBoardEnable(int board, bool enabled) {
    if (board < 0 || board >= DRIVER_BOARD_COUNT) return false;
    // ~EN is active LOW on the TMC2209: enabled -> LOW, disabled -> HIGH
    bool levelHigh = DRIVERS_EN_ACTIVE_LOW ? !enabled : enabled;
    if (!driversPcf.writeBit(DRIVER_EN_PCF_BIT[board], levelHigh)) {
        Serial.printf("[STEPPER] ERROR: cannot %s driver board %d (PCF8574)\n",
                      enabled ? "enable" : "disable", board);
        return false;
    }
    return true;
}

void stepperMotorsEnable(bool enabled) {
    // Both boards in one I2C write
    uint8_t mask = 0, value = 0;
    bool levelHigh = DRIVERS_EN_ACTIVE_LOW ? !enabled : enabled;
    for (int board = 0; board < DRIVER_BOARD_COUNT; board++) {
        mask |= (1u << DRIVER_EN_PCF_BIT[board]);
        if (levelHigh) value |= (1u << DRIVER_EN_PCF_BIT[board]);
    }
    if (!driversPcf.writeMasked(mask, value)) {
        Serial.printf("[STEPPER] ERROR: cannot %s the motors (PCF8574)\n", enabled ? "enable" : "disable");
        return;
    }
    Serial.println(enabled ? "[STEPPER] Motors enabled" : "[STEPPER] Motors disabled");
}

long stepperSlotStepsPerRev(int slot) {
    if (slot < 0 || slot >= STEPPER_SLOT_COUNT) return 0;
    return (long)STEPPER_SLOTS[slot].stepsPerRev * STEPPER_SLOTS[slot].microsteps;
}
