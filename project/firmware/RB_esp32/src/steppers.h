// Shared FastAccelStepper engine and slot wiring.
// Wheels and stepper actuators both get their motor from connectStepperSlot(),
// which applies the line of STEPPER_SLOTS (robot_config.h) for that slot.
//
// FastAccelStepper generates the STEP pulses in hardware (ESP32 RMT/MCPWM peripherals)
// with acceleration ramps: move()/moveTo() return immediately and the motor runs in the
// background. Use isRunning() to know when it has arrived.
//
// ~EN and MS1/MS2 are on the body PCF8574 (I2C), one set per driver board:
// Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN) must be called BEFORE steppersBegin().
// steppersBegin() writes the microsteps of STEPPER_SLOTS to MS1/MS2; both slots of a
// driver board (0+1, 2+3) must therefore have the same microsteps.
//
// Units: everything here is in MICROSTEPS (full steps x microsteps), speeds in microsteps/s.
// StepperActuator (actuators.h) converts them to mm or degrees.

#pragma once

#include <FastAccelStepper.h>

// Call once in setup(), after Wire.begin() and before any connectStepperSlot().
// Leaves the drivers disabled.
void steppersBegin();

// Returns the configured motor of the slot, or nullptr if the slot config is invalid
// (a [CONFIG] error explains why). Calling it twice for the same slot returns the same motor.
// The motor gets the slot's direction, maxSpeedHz and accelerationHz2 from STEPPER_SLOTS.
FastAccelStepper* connectStepperSlot(int slot);

// ~EN of both driver boards: true = motors powered and holding.
// false = coils unpowered: the motors turn freely by hand and no longer hold their position.
void stepperMotorsEnable(bool enabled);

// ~EN of one driver board only (0 = NP1, slots 0-1 / 1 = NP2, slots 2-3).
// The wheels are on slots 1 and 3, one per board: disabling one board stops one wheel.
// Returns false if the PCF8574 did not answer.
bool stepperBoardEnable(int board, bool enabled);

// Microsteps per motor revolution for the slot (stepsPerRev * microsteps).
// Returns 0 for a slot that does not exist.
long stepperSlotStepsPerRev(int slot);
