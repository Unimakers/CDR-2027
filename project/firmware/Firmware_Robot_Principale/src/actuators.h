// Actuators: stepper (slots 2..5), servo and pump (PCA9685 connectors 1..32).
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
class StepperActuator {
public:
    // unitsPerMotorRev: mechanism travel per motor turn, in the unit used by strategies.
    // Belt: pulley teeth x belt pitch, in mm (20 x 2 mm GT2 = 40). Rotating arm: degrees.
    StepperActuator(const char* name, int slot, float unitsPerMotorRev);

    bool begin(); // called by actuatorsBegin()

    bool moveTo(float position);      // blocking, returns false if interrupted
    void startMoveTo(float position); // non-blocking, check isDone()
    bool isDone();

    // No limit switch: drive `travel` units (sign = direction) slowly into the
    // mechanical end stop, then call that position 0. travel must exceed the full stroke.
    void homeAgainstStop(float travel);
    void setZero();

    float getPosition();
    void stop();

    const char* getName() const { return _name; }
    int getSlot() const { return _slot; }

private:
    const char* _name;
    int _slot;
    float _unitsPerMotorRev;
    float _stepsPerUnit;
    FastAccelStepper* _stepper;

    bool canRun(const char* command);
    bool waitUntilDone();
};

// ---------------------------------------------------------------------------
class ServoActuator {
public:
    ServoActuator(const char* name, int connector,
                  int minPulseUs = SERVO_DEFAULT_MIN_PULSE_US,
                  int maxPulseUs = SERVO_DEFAULT_MAX_PULSE_US);

    bool begin(); // called by actuatorsBegin()

    void setAngle(float angleDeg); // 0..180, returns immediately: use matchWait() to let it move
    void release();                // stop the pulses, the servo goes limp
    float getAngle() const { return _angleDeg; }

    const char* getName() const { return _name; }
    int getConnector() const { return _connector; }

private:
    const char* _name;
    int _connector;
    int _minPulseUs;
    int _maxPulseUs;
    int _pca;
    int _channel;
    float _angleDeg;
    bool _ready;
};

// ---------------------------------------------------------------------------
// Any on/off load behind a MOSFET: pump, valve, electromagnet...
class PumpActuator {
public:
    PumpActuator(const char* name, int connector);

    bool begin(); // called by actuatorsBegin()

    void on();
    void off();
    bool isOn() const { return _on; }

    void forceOff(); // works even after match end, used by actuatorsStopAll()

    const char* getName() const { return _name; }
    int getConnector() const { return _connector; }

private:
    const char* _name;
    int _connector;
    int _pca;
    int _channel;
    bool _on;
    bool _ready;

    void write(bool on);
};
