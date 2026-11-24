#pragma once

#include <FastAccelStepper.h>

extern FastAccelStepperEngine engine;
extern FastAccelStepper *leftmotor;
extern FastAccelStepper *rightmotor;

void WaitForMotors();
void go(float distanceMm);
void turn(float angle);
float setAngle(float start_angleDeg, float wanted_angleDeg);