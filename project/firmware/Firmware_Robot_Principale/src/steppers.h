// Shared FastAccelStepper engine and slot wiring.
// Wheels and stepper actuators both get their motor from connectStepperSlot(),
// which applies the line of STEPPER_SLOTS (robot_config.h) for that slot.
//
// FastAccelStepper generates the STEP pulses in hardware (ESP32 RMT/MCPWM peripherals)
// with acceleration ramps: move()/moveTo() return immediately and the motor runs in the
// background. Use isRunning() to know when it has arrived.
//
// Units: everything here is in MICROSTEPS (full steps x microsteps), speeds in microsteps/s.
// StepperActuator (actuators.h) converts them to mm or degrees.

#pragma once

#include <FastAccelStepper.h>

// Call once in setup(), before any connectStepperSlot(). Leaves the drivers disabled.
void steppersBegin();

// Returns the configured motor of the slot, or nullptr if the slot config is invalid
// (a [CONFIG] error explains why). Calling it twice for the same slot returns the same motor.
// The motor gets the slot's direction, maxSpeedHz and accelerationHz2 from STEPPER_SLOTS.
FastAccelStepper* connectStepperSlot(int slot);

// Shared ~EN of the 4 TMC2209: true = motors powered and holding.
// false = coils unpowered: the motors turn freely by hand and no longer hold their position.
void stepperMotorsEnable(bool enabled);

// Microsteps per motor revolution for the slot (stepsPerRev * microsteps).
// Returns 0 for a slot that does not exist.
long stepperSlotStepsPerRev(int slot);
