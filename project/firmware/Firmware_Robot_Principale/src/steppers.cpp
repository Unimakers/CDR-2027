#include <Arduino.h>
#include "config.h"
#include "steppers.h"

static FastAccelStepperEngine engine;
static FastAccelStepper* slotSteppers[STEPPER_SLOT_COUNT] = {nullptr};

void steppersBegin() {
    engine.init();

    if (MOTORS_EN_PIN >= 0) {
        pinMode(MOTORS_EN_PIN, OUTPUT);
        stepperMotorsEnable(false);
    } else {
        Serial.println("[CONFIG] MOTORS_EN_PIN not set: drivers stay enabled at all times");
    }
    Serial.println("[STEPPER] Engine initialized");
}

static bool isSupportedMicrostep(int microsteps) {
    // TMC2209 standalone mode, see the jumper table in board_pins.h
    return microsteps == 2 || microsteps == 4 || microsteps == 8 || microsteps == 16;
}

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
    }
    if (cfg.maxSpeedHz <= 0 || cfg.accelerationHz2 <= 0) {
        Serial.printf("[CONFIG] Slot %d: maxSpeedHz / accelerationHz2 not set\n", slot);
        ok = false;
    }
    return ok;
}

FastAccelStepper* connectStepperSlot(int slot) {
    if (!checkSlotConfig(slot)) return nullptr;
    if (slotSteppers[slot]) return slotSteppers[slot];

    const StepperSlot& cfg = STEPPER_SLOTS[slot];
    FastAccelStepper* stepper = engine.stepperConnectToPin(cfg.stepPin);
    if (!stepper) {
        Serial.printf("[STEPPER] ERROR: slot %d, GPIO%d cannot be used as a STEP pin\n", slot, cfg.stepPin);
        return nullptr;
    }
    stepper->setDirectionPin(cfg.dirPin, !cfg.inverted);
    // ~EN is shared by the 6 drivers and handled by stepperMotorsEnable()
    stepper->setAutoEnable(false);
    stepper->setSpeedInHz(cfg.maxSpeedHz);
    stepper->setAcceleration(cfg.accelerationHz2);

    slotSteppers[slot] = stepper;
    Serial.printf("[STEPPER] Slot %d connected (STEP GPIO%d, DIR GPIO%d)\n", slot, cfg.stepPin, cfg.dirPin);
    return stepper;
}

void stepperMotorsEnable(bool enabled) {
    if (MOTORS_EN_PIN < 0) return;
    bool levelHigh = MOTORS_EN_ACTIVE_LOW ? !enabled : enabled;
    digitalWrite(MOTORS_EN_PIN, levelHigh ? HIGH : LOW);
    Serial.println(enabled ? "[STEPPER] Motors enabled" : "[STEPPER] Motors disabled");
}

long stepperSlotStepsPerRev(int slot) {
    if (slot < 0 || slot >= STEPPER_SLOT_COUNT) return 0;
    return (long)STEPPER_SLOTS[slot].stepsPerRev * STEPPER_SLOTS[slot].microsteps;
}
