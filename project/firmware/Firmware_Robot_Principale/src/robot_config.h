// Robot configuration: what each board slot is used for, and the tuning values.
// Pins come from board_pins.h; this file changes every time an actuator is tried.
// -1 = placeholder, not tuned yet.

#pragma once

#include <stdint.h>
#include "board_pins.h"

// Serial debug output
#define DEBUG_SERIAL true

// match duration
#define MATCH_DURATION 100000UL // in milliseconds (100s)

// ---------------------------------------------------------------------------
// Stepper slots: one line per TMC2209 slot (see the slot table in board_pins.h)
// ---------------------------------------------------------------------------
struct StepperSlot {
    int  stepPin;
    int  dirPin;
    bool inverted;        // true if the motor turns the wrong way
    int  stepsPerRev;     // full steps per motor revolution (200 for a 1.8 deg motor)
    int  microsteps;      // must match the slot's MS1/MS2 solder jumpers
    int  maxSpeedHz;      // for setSpeedInHz(), in steps/s
    int  accelerationHz2; // for setAcceleration(), in steps/s^2
};

const StepperSlot STEPPER_SLOTS[STEPPER_SLOT_COUNT] = {
    // stepPin     dirPin     inverted  stepsPerRev  microsteps  maxSpeedHz  accelerationHz2
    {STEP_1_PIN, DIR_1_PIN, false,    -1,          -1,         -1,         -1}, // slot 0
    {STEP_2_PIN, DIR_2_PIN, false,    -1,          -1,         -1,         -1}, // slot 1
    {STEP_3_PIN, DIR_3_PIN, false,    -1,          -1,         -1,         -1}, // slot 2
    {STEP_4_PIN, DIR_4_PIN, false,    -1,          -1,         -1,         -1}, // slot 3
    {STEP_5_PIN, DIR_5_PIN, false,    -1,          -1,         -1,         -1}, // slot 4
    {STEP_6_PIN, DIR_6_PIN, false,    -1,          -1,         -1,         -1}, // slot 5
};

// ---------------------------------------------------------------------------
// Wheels: always on slots 0 and 1. Slots 2..5 are free for actuators.
// ---------------------------------------------------------------------------
#define WHEEL_LEFT_SLOT 0
#define WHEEL_RIGHT_SLOT 1

static_assert(WHEEL_LEFT_SLOT == 0 || WHEEL_LEFT_SLOT == 1, "WHEEL_LEFT_SLOT must be 0 or 1");
static_assert(WHEEL_RIGHT_SLOT == 0 || WHEEL_RIGHT_SLOT == 1, "WHEEL_RIGHT_SLOT must be 0 or 1");
static_assert(WHEEL_LEFT_SLOT != WHEEL_RIGHT_SLOT, "Both wheels cannot use the same slot");

// ---------------------------------------------------------------------------
// Stepper actuators (slots 2..5): no limit switch, homing is done by driving
// into the mechanical end stop at a fraction of the slot's maxSpeedHz.
// ---------------------------------------------------------------------------
#define ACTUATOR_HOMING_SPEED_PERCENT 20

// ---------------------------------------------------------------------------
// PCA9685 PWM (servos and pumps)
// The frequency is shared by the 16 channels of a chip. 50 Hz is what servos expect;
// pumps only need fully ON / fully OFF, which works at any frequency.
// ---------------------------------------------------------------------------
#define PCA_PWM_FREQUENCY_HZ 50
#define PCA_OSCILLATOR_HZ 25000000 // nominal; measure and adjust if servo angles are off

// Default servo pulse range, can be overridden per servo in the actuator config
#define SERVO_DEFAULT_MIN_PULSE_US 500  // pulse width at 0 deg
#define SERVO_DEFAULT_MAX_PULSE_US 2500 // pulse width at 180 deg

// ---------------------------------------------------------------------------
// LIDAR LD06 filtering (see setDistanceRange / setAngleRange / setIntensityThreshold)
// Set real values before calling enableFiltering(): the library stores them unsigned,
// so -1 would turn into 65535 / 255 and filter out every point.
// ---------------------------------------------------------------------------
#define LD06_MIN_DISTANCE -1        // in mm
#define LD06_MAX_DISTANCE -1        // in mm
#define LD06_MIN_ANGLE -1           // in degrees
#define LD06_MAX_ANGLE -1           // in degrees
#define LD06_INTENSITY_THRESHOLD -1 // 0-255
#define LD06_UPSIDE_DOWN false

// Lidar position relative to the robot center (see setOffsetPosition)
#define LD06_OFFSET_X -1     // in mm
#define LD06_OFFSET_Y -1     // in mm
#define LD06_OFFSET_ANGLE -1 // in degrees
