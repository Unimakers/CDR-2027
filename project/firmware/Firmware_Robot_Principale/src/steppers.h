// Shared FastAccelStepper engine and slot wiring.
// Wheels and stepper actuators both get their motor from connectStepperSlot(),
// which applies the line of STEPPER_SLOTS (robot_config.h) for that slot.

#pragma once

#include <FastAccelStepper.h>

// Call once in setup(), before any connectStepperSlot(). Leaves the drivers disabled.
void steppersBegin();

// Returns the configured motor of the slot, or nullptr if the slot config is invalid
// (a [CONFIG] error explains why). Calling it twice for the same slot returns the same motor.
FastAccelStepper* connectStepperSlot(int slot);

// Shared ~EN of the 6 TMC2209: true = motors powered and holding.
void stepperMotorsEnable(bool enabled);

// Microsteps per motor revolution for the slot (stepsPerRev * microsteps).
long stepperSlotStepsPerRev(int slot);
