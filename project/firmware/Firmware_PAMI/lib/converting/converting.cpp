// Bibliothèques inclus dans PIO + built-in
#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>

// Bibliothèques créées
#include <config.h>
#include <converting.h>

float x = START_X;
float y = START_Y;
float angle = START_ANGLE;

float DistanceToStep(float distanceMm) // converti une distance (en mm) en Steps
{
  return lroundf((distanceMm / wheelCircumferenceMm * stepsPerWheelRevolution)/2);
}

float StepToDistance(float step)
{
    return lround((2*step*wheelCircumferenceMm)/stepsPerWheelRevolution);
}

float AngleToStep(float angleDeg)  // converti un angle (en degré) en Steps
{
  float wheelTravelMm = (angleDeg * PI / 180.0f) * (WHEEL_BASE_MM / 4);
  return lroundf(wheelTravelMm / wheelCircumferenceMm * stepsPerWheelRevolution);
}

float update_x(float x, float angleDeg, float distanceMm)
{
    return lround(x + cos(angleDeg)*distanceMm);
}

float uptade_y(float y, float angleDeg, float distanceMm)
{
    return lround(y + sin(angleDeg)*distanceMm);
}

float updateAngle(float starting_angleDeg, float turn_angleDeg)
{
    return lround(starting_angleDeg + turn_angleDeg);
}