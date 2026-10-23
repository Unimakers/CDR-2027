// Stepper slots, see steppers.h. One FastAccelStepper engine drives every slot;
// each slot is connected at most once and kept in slotSteppers[].

#include <Arduino.h>
#include "config.h"
#include "steppers.h"

static FastAccelStepperEngine engine;
// Motor already connected on each slot (nullptr = not connected yet)
static FastAccelStepper* slotSteppers[STEPPER_SLOT_COUNT] = {nullptr};

void steppersBegin() {
    engine.init();

    // The ~EN line floats until this point (no pull resistor on the board):
    // drive it right away so the drivers start disabled.
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
    // ~EN is shared by the 4 drivers and handled by stepperMotorsEnable()
    stepper->setAutoEnable(false);
    stepper->setSpeedInHz(cfg.maxSpeedHz);
    stepper->setAcceleration(cfg.accelerationHz2);

    slotSteppers[slot] = stepper;
    Serial.printf("[STEPPER] Slot %d connected (STEP GPIO%d, DIR GPIO%d)\n", slot, cfg.stepPin, cfg.dirPin);
    return stepper;
}

void stepperMotorsEnable(bool enabled) {
    if (MOTORS_EN_PIN < 0) return;
    // ~EN is active LOW on the TMC2209: enabled -> LOW, disabled -> HIGH
    bool levelHigh = MOTORS_EN_ACTIVE_LOW ? !enabled : enabled;
    digitalWrite(MOTORS_EN_PIN, levelHigh ? HIGH : LOW);
    Serial.println(enabled ? "[STEPPER] Motors enabled" : "[STEPPER] Motors disabled");
}

long stepperSlotStepsPerRev(int slot) {
    if (slot < 0 || slot >= STEPPER_SLOT_COUNT) return 0;
    return (long)STEPPER_SLOTS[slot].stepsPerRev * STEPPER_SLOTS[slot].microsteps;
}
