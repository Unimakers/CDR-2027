#pragma once

#include <ESP32Servo.h>
#include <FastAccelStepper.h>

extern FastAccelStepperEngine engine;
extern FastAccelStepper *leftmotor;
extern FastAccelStepper *rightmotor;

extern Servo sword;
extern Servo catapult;

float DistanceToStep(float distanceMm);
float AngleToStep(float angleDeg);
void WaitForMotors();
void go(float distanceMm);
void turn(float angle);
