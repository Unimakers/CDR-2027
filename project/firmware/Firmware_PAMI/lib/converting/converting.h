#pragma once

#include <FastAccelStepper.h>

float DistanceToStep(float distanceMm);
float StepToDistance(float step);
float AngleToStep(float angleDeg);
float update_x(float x, float angleDeg, float distanceMm);
float uptade_y(float y, float angleDeg, float distanceMm);
float updateAngle(float starting_angleDeg, float turn_angleDeg);
