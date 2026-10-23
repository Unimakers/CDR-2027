// Actuators: stepper (slots 2..3), servo and pump (PCA9685 connectors 1..32).
//
// Adding an actuator = declaring one global object, anywhere (e.g. at the top of the
// strategy file). It registers itself; actuatorsBegin() checks and starts all of them.
//
//   StepperActuator elevator("elevator", 2, 40.0f);  // slot 2, belt: 20-tooth GT2 pulley = 40 mm per motor turn
//   ServoActuator   arm("arm", 1);                   // connector 1, default pulse range
//   PumpActuator    suction("suction", 17);          // connector 17
//
//   elevator.homeAgainstStop(-200);   // before the match: drive down into the end stop
//   elevator.moveTo(120);             // blocking, in mm
//   arm.setAngle(90);
//   suction.on();
//   matchWait(300);
//
// Every command is ignored once the match is over.
// Setup order in main: Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN); steppersBegin(); actuatorsBegin();
// The stepper actuators only move once stepperMotorsEnable(true) has been called.
//
// Error handling: a misconfigured actuator (wrong slot/connector, PCA9685 not answering...)
// prints a [CONFIG] or [ACT] line at startup and then ignores its commands, with a log line,
// instead of crashing the robot. Always read the serial output after changing the config.

#pragma once

#include <Arduino.h>
#include <FastAccelStepper.h>
#include "config.h"

// Starts the PCA9685s and every declared actuator. Returns false if anything is
// misconfigured ([CONFIG] / [ACT] logs say what); the valid actuators still work.
bool actuatorsBegin();

// Match end: stops the stepper actuators and switches the pumps off.
// Servos keep their last position.
void actuatorsStopAll();

// ---------------------------------------------------------------------------
// Stepper motor on a TMC2209 slot (2..3, slots 0 and 1 are the wheels), driving a
// mechanism (lift, rotating arm...). Positions are in the mechanism unit (mm, degrees...),
// 0 being the position found by homeAgainstStop() (or the position at startup).
class StepperActuator {
public:
    // name: shown in the logs. slot: line of STEPPER_SLOTS in robot_config.h.
    // unitsPerMotorRev: mechanism travel per motor turn, in the unit used by strategies.
    // Belt: pulley teeth x belt pitch, in mm (20 x 2 mm GT2 = 40). Rotating arm: degrees
    // (360 for a direct drive, 360 / ratio behind a gearbox).
    StepperActuator(const char* name, int slot, float unitsPerMotorRev);

    bool begin(); // called by actuatorsBegin()

    // Absolute moves, at the slot's maxSpeedHz with its acceleration ramp.
    bool moveTo(float position);      // blocking, returns false if interrupted by the match end
    void startMoveTo(float position); // non-blocking, check isDone() (or do something else meanwhile)
    bool isDone();                    // true when the motor is stopped (or not initialized)

    // No limit switch: drive `travel` units (sign = direction) slowly into the
    // mechanical end stop, then call that position 0. travel must exceed the full stroke.
    // The speed is ACTUATOR_HOMING_SPEED_PERCENT of maxSpeedHz (robot_config.h): the motor
    // skips steps against the stop instead of forcing on the mechanism.
    void homeAgainstStop(float travel);
    void setZero(); // the current position becomes 0, without moving

    float getPosition(); // current position in units, updated while the motor moves
    void stop();         // immediate stop, no deceleration ramp

    const char* getName() const { return _name; }
    int getSlot() const { return _slot; }

private:
    const char* _name;
    int _slot;
    float _unitsPerMotorRev;
    float _stepsPerUnit;        // microsteps per unit, computed in begin()
    FastAccelStepper* _stepper; // nullptr until begin() succeeds

    bool canRun(const char* command); // false (with a log) if not initialized or match over
    bool waitUntilDone();             // waits for the end of the move, stops it at match end
};

// ---------------------------------------------------------------------------
// Hobby servo on one of the 32 actuator connectors. The PCA9685 keeps sending the pulse
// (50 Hz, PCA_PWM_FREQUENCY_HZ), so the servo holds its angle without any CPU work.
class ServoActuator {
public:
    // minPulseUs / maxPulseUs: pulse widths at 0 and 180 degrees. The defaults
    // (robot_config.h) fit most servos; narrow them if the servo hits its own stops.
    ServoActuator(const char* name, int connector,
                  int minPulseUs = SERVO_DEFAULT_MIN_PULSE_US,
                  int maxPulseUs = SERVO_DEFAULT_MAX_PULSE_US);

    bool begin(); // called by actuatorsBegin()

    void setAngle(float angleDeg); // 0..180, returns immediately: use matchWait() to let it move
    void release();                // stop the pulses, the servo goes limp
    float getAngle() const { return _angleDeg; } // last commanded angle, -1 before the first setAngle()

    const char* getName() const { return _name; }
    int getConnector() const { return _connector; }

private:
    const char* _name;
    int _connector; // 1..32, as printed on the board
    int _minPulseUs;
    int _maxPulseUs;
    int _pca;       // 0 or 1, from the connector number
    int _channel;   // PCA9685 output 0..15, from ACTUATOR_PCA_CHANNEL
    float _angleDeg;
    bool _ready;    // false if begin() failed: every command is then ignored
};

// ---------------------------------------------------------------------------
// Any on/off load behind a MOSFET: pump, valve, electromagnet...
// The PCA9685 output is held fully HIGH (on) or fully LOW (off), no PWM.
class PumpActuator {
public:
    PumpActuator(const char* name, int connector);

    bool begin(); // called by actuatorsBegin(), switches the output off

    void on();  // ignored once the match is over
    void off(); // always allowed
    bool isOn() const { return _on; }

    void forceOff(); // works even after match end, used by actuatorsStopAll()

    const char* getName() const { return _name; }
    int getConnector() const { return _connector; }

private:
    const char* _name;
    int _connector; // 1..32, as printed on the board
    int _pca;       // 0 or 1, from the connector number
    int _channel;   // PCA9685 output 0..15, from ACTUATOR_PCA_CHANNEL
    bool _on;
    bool _ready;    // false if begin() failed: every command is then ignored

    void write(bool on); // sets the output and _on, no checks, no log
};
