#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <math.h>
#include "actuators.h"
#include "steppers.h"
#include "match.h"

// ---------------------------------------------------------------------------
// PCA9685 drivers
// ---------------------------------------------------------------------------
static_assert(PCA_COUNT == 2, "Update pcaDrivers[] and PCA_ADDRESSES[] to match PCA_COUNT");

static const int PCA_ADDRESSES[PCA_COUNT] = {PCA_1_ADDR, PCA_2_ADDR};
static Adafruit_PWMServoDriver pcaDrivers[PCA_COUNT] = {
    Adafruit_PWMServoDriver((uint8_t)PCA_1_ADDR, Wire),
    Adafruit_PWMServoDriver((uint8_t)PCA_2_ADDR, Wire),
};
static bool pcaReady[PCA_COUNT] = {false, false};

#define PCA_FULL_ON 4095 // setPin(): 4095 = constant HIGH output

static bool beginPca(int index) {
    int address = PCA_ADDRESSES[index];
    if (address < 0) {
        Serial.printf("[CONFIG] PCA_%d_ADDR not set in board_pins.h\n", index + 1);
        return false;
    }
    if (!pcaDrivers[index].begin()) {
        Serial.printf("[ACT] ERROR: no PCA9685 answering at 0x%02X\n", address);
        return false;
    }
    pcaDrivers[index].setOscillatorFrequency(PCA_OSCILLATOR_HZ);
    pcaDrivers[index].setPWMFreq(PCA_PWM_FREQUENCY_HZ);
    for (int channel = 0; channel < PCA_CHANNEL_COUNT; channel++) {
        pcaDrivers[index].setPin(channel, 0);
    }
    Serial.printf("[ACT] PCA9685 #%d ready at 0x%02X, %d Hz\n", index + 1, address, PCA_PWM_FREQUENCY_HZ);
    return true;
}

// Connector n (1..32) -> PCA index and channel, see board_pins.h
static bool findPcaChannel(const char* name, int connector, int* pca, int* channel) {
    if (connector < 1 || connector > ACTUATOR_CONNECTOR_COUNT) {
        Serial.printf("[CONFIG] %s: connector %d does not exist (1..%d)\n",
                      name, connector, ACTUATOR_CONNECTOR_COUNT);
        return false;
    }
    *pca = (connector - 1) / PCA_CHANNEL_COUNT;
    *channel = (connector - 1) % PCA_CHANNEL_COUNT;
    if (!pcaReady[*pca]) {
        Serial.printf("[ACT] %s: PCA9685 #%d not available\n", name, *pca + 1);
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Registry: each actuator adds itself in its constructor.
// Plain arrays are zero-initialized before any constructor runs, so global
// actuators declared in any file can register safely.
// ---------------------------------------------------------------------------
static StepperActuator* stepperActuators[STEPPER_SLOT_COUNT];
static int stepperActuatorCount = 0;
static ServoActuator* servoActuators[ACTUATOR_CONNECTOR_COUNT];
static int servoActuatorCount = 0;
static PumpActuator* pumpActuators[ACTUATOR_CONNECTOR_COUNT];
static int pumpActuatorCount = 0;
static bool registryFull = false;

static bool isConnectorUsedTwice(int connector) {
    int uses = 0;
    for (int i = 0; i < servoActuatorCount; i++) {
        if (servoActuators[i]->getConnector() == connector) uses++;
    }
    for (int i = 0; i < pumpActuatorCount; i++) {
        if (pumpActuators[i]->getConnector() == connector) uses++;
    }
    return uses > 1;
}

static bool checkActuatorConflicts() {
    bool ok = true;
    for (int i = 0; i < stepperActuatorCount; i++) {
        int slot = stepperActuators[i]->getSlot();
        if (slot == WHEEL_LEFT_SLOT || slot == WHEEL_RIGHT_SLOT) {
            Serial.printf("[CONFIG] %s: slot %d is used by a wheel\n", stepperActuators[i]->getName(), slot);
            ok = false;
        }
        for (int j = i + 1; j < stepperActuatorCount; j++) {
            if (stepperActuators[j]->getSlot() == slot) {
                Serial.printf("[CONFIG] %s and %s both use slot %d\n",
                              stepperActuators[i]->getName(), stepperActuators[j]->getName(), slot);
                ok = false;
            }
        }
    }
    for (int connector = 1; connector <= ACTUATOR_CONNECTOR_COUNT; connector++) {
        if (isConnectorUsedTwice(connector)) {
            Serial.printf("[CONFIG] Connector %d is used by several actuators\n", connector);
            ok = false;
        }
    }
    return ok;
}

bool actuatorsBegin() {
    bool ok = true;

    if (PCA_OE_PIN >= 0) {
        pinMode(PCA_OE_PIN, OUTPUT);
        digitalWrite(PCA_OE_PIN, HIGH); // outputs off while configuring
    }
    for (int i = 0; i < PCA_COUNT; i++) {
        pcaReady[i] = beginPca(i);
    }
    if (PCA_OE_PIN >= 0) {
        digitalWrite(PCA_OE_PIN, LOW);
    }

    if (registryFull) {
        Serial.println("[CONFIG] Too many actuators declared, some were ignored");
        ok = false;
    }
    if (!checkActuatorConflicts()) ok = false;

    for (int i = 0; i < stepperActuatorCount; i++) {
        if (!stepperActuators[i]->begin()) ok = false;
    }
    for (int i = 0; i < servoActuatorCount; i++) {
        if (!servoActuators[i]->begin()) ok = false;
    }
    for (int i = 0; i < pumpActuatorCount; i++) {
        if (!pumpActuators[i]->begin()) ok = false;
    }

    Serial.printf("[ACT] %d stepper(s), %d servo(s), %d pump(s) declared%s\n",
                  stepperActuatorCount, servoActuatorCount, pumpActuatorCount,
                  ok ? "" : " - WITH ERRORS");
    return ok;
}

void actuatorsStopAll() {
    for (int i = 0; i < stepperActuatorCount; i++) {
        stepperActuators[i]->stop();
    }
    for (int i = 0; i < pumpActuatorCount; i++) {
        pumpActuators[i]->forceOff();
    }
    Serial.println("[ACT] All actuators stopped");
}

// ---------------------------------------------------------------------------
// StepperActuator
// ---------------------------------------------------------------------------
StepperActuator::StepperActuator(const char* name, int slot, float unitsPerMotorRev)
    : _name(name), _slot(slot), _unitsPerMotorRev(unitsPerMotorRev),
      _stepsPerUnit(0), _stepper(nullptr) {
    if (stepperActuatorCount < STEPPER_SLOT_COUNT) {
        stepperActuators[stepperActuatorCount++] = this;
    } else {
        registryFull = true;
    }
}

bool StepperActuator::begin() {
    if (_unitsPerMotorRev <= 0) {
        Serial.printf("[CONFIG] %s: unitsPerMotorRev must be > 0\n", _name);
        return false;
    }
    if (_slot == WHEEL_LEFT_SLOT || _slot == WHEEL_RIGHT_SLOT) return false;

    _stepper = connectStepperSlot(_slot);
    if (!_stepper) {
        Serial.printf("[ACT] %s: slot %d unusable\n", _name, _slot);
        return false;
    }
    _stepsPerUnit = stepperSlotStepsPerRev(_slot) / _unitsPerMotorRev;
    return true;
}

bool StepperActuator::canRun(const char* command) {
    if (!_stepper) {
        Serial.printf("[ACT] %s: %s ignored, actuator not initialized\n", _name, command);
        return false;
    }
    if (matchHasEnded()) {
        Serial.printf("[ACT] %s: %s ignored, match is over\n", _name, command);
        return false;
    }
    return true;
}

bool StepperActuator::waitUntilDone() {
    while (_stepper->isRunning()) {
        if (matchHasEnded()) {
            _stepper->forceStop();
            return false;
        }
        delay(1);
    }
    return true;
}

void StepperActuator::startMoveTo(float position) {
    if (!canRun("moveTo")) return;
    Serial.printf("[ACT] %s moveTo %.1f\n", _name, position);
    MoveResultCode result = _stepper->moveTo(lroundf(position * _stepsPerUnit));
    if (result != MOVE_OK) {
        Serial.printf("[ACT] %s: moveTo refused by FastAccelStepper (code %d)\n", _name, (int)result);
    }
}

bool StepperActuator::moveTo(float position) {
    if (!canRun("moveTo")) return false;
    startMoveTo(position);
    return waitUntilDone();
}

bool StepperActuator::isDone() {
    return !_stepper || !_stepper->isRunning();
}

void StepperActuator::homeAgainstStop(float travel) {
    if (!canRun("homeAgainstStop")) return;
    Serial.printf("[ACT] %s homing against end stop (%.1f)\n", _name, travel);

    const StepperSlot& cfg = STEPPER_SLOTS[_slot];
    _stepper->setSpeedInHz(cfg.maxSpeedHz * ACTUATOR_HOMING_SPEED_PERCENT / 100);
    _stepper->move(lroundf(travel * _stepsPerUnit));
    bool completed = waitUntilDone();
    _stepper->setSpeedInHz(cfg.maxSpeedHz);

    if (completed) setZero();
}

void StepperActuator::setZero() {
    if (!_stepper) return;
    _stepper->setCurrentPosition(0);
    Serial.printf("[ACT] %s position set to 0\n", _name);
}

float StepperActuator::getPosition() {
    if (!_stepper || _stepsPerUnit == 0) return 0;
    return _stepper->getCurrentPosition() / _stepsPerUnit;
}

void StepperActuator::stop() {
    if (_stepper) _stepper->forceStop();
}

// ---------------------------------------------------------------------------
// ServoActuator
// ---------------------------------------------------------------------------
ServoActuator::ServoActuator(const char* name, int connector, int minPulseUs, int maxPulseUs)
    : _name(name), _connector(connector), _minPulseUs(minPulseUs), _maxPulseUs(maxPulseUs),
      _pca(-1), _channel(-1), _angleDeg(-1), _ready(false) {
    if (servoActuatorCount < ACTUATOR_CONNECTOR_COUNT) {
        servoActuators[servoActuatorCount++] = this;
    } else {
        registryFull = true;
    }
}

bool ServoActuator::begin() {
    if (_minPulseUs <= 0 || _maxPulseUs <= _minPulseUs) {
        Serial.printf("[CONFIG] %s: invalid pulse range %d..%d us\n", _name, _minPulseUs, _maxPulseUs);
        return false;
    }
    _ready = findPcaChannel(_name, _connector, &_pca, &_channel);
    return _ready;
}

void ServoActuator::setAngle(float angleDeg) {
    if (!_ready) {
        Serial.printf("[ACT] %s: setAngle ignored, actuator not initialized\n", _name);
        return;
    }
    if (matchHasEnded()) {
        Serial.printf("[ACT] %s: setAngle ignored, match is over\n", _name);
        return;
    }
    angleDeg = constrain(angleDeg, 0.0f, 180.0f);
    int pulseUs = _minPulseUs + lroundf((_maxPulseUs - _minPulseUs) * angleDeg / 180.0f);
    pcaDrivers[_pca].writeMicroseconds(_channel, pulseUs);
    _angleDeg = angleDeg;
    Serial.printf("[ACT] %s angle %.0f (%d us)\n", _name, angleDeg, pulseUs);
}

void ServoActuator::release() {
    if (!_ready) return;
    pcaDrivers[_pca].setPin(_channel, 0);
    Serial.printf("[ACT] %s released\n", _name);
}

// ---------------------------------------------------------------------------
// PumpActuator
// ---------------------------------------------------------------------------
PumpActuator::PumpActuator(const char* name, int connector)
    : _name(name), _connector(connector), _pca(-1), _channel(-1), _on(false), _ready(false) {
    if (pumpActuatorCount < ACTUATOR_CONNECTOR_COUNT) {
        pumpActuators[pumpActuatorCount++] = this;
    } else {
        registryFull = true;
    }
}

bool PumpActuator::begin() {
    _ready = findPcaChannel(_name, _connector, &_pca, &_channel);
    if (_ready) write(false);
    return _ready;
}

void PumpActuator::write(bool on) {
    pcaDrivers[_pca].setPin(_channel, on ? PCA_FULL_ON : 0);
    _on = on;
}

void PumpActuator::on() {
    if (!_ready) {
        Serial.printf("[ACT] %s: on ignored, actuator not initialized\n", _name);
        return;
    }
    if (matchHasEnded()) {
        Serial.printf("[ACT] %s: on ignored, match is over\n", _name);
        return;
    }
    write(true);
    Serial.printf("[ACT] %s on\n", _name);
}

void PumpActuator::off() {
    if (!_ready) return;
    write(false);
    Serial.printf("[ACT] %s off\n", _name);
}

void PumpActuator::forceOff() {
    if (_ready) write(false);
}
