#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <FastAccelStepper.h>
#include <ESP32Servo.h>
#include <TMCStepper.h>
#include <Wire.h>
#include "config.h"
#include "functions.h"


FastAccelStepperEngine engine;
FastAccelStepper *leftmotor = nullptr;
FastAccelStepper *rightmotor = nullptr;

Servo sword;
Servo catapult;

float DistanceToStep(float distanceMm) // converti une distance (en mm) en Steps
{
  return lroundf((distanceMm / wheelCircumferenceMm *stepsPerWheelRevolution)/2);
}

float AngleToStep(float angleDeg)  // converti un angle (en degré) en Steps
{
  float wheelTravelMm = (angleDeg * PI / 180.0f) * (WHEEL_BASE_MM / 4);
  return lroundf(wheelTravelMm / wheelCircumferenceMm * stepsPerWheelRevolution);
}

void WaitForMotors() // attend que les moteurs soient arrêtés
{
  while (leftmotor->isRunning() || rightmotor->isRunning()) {
    delay(1);
  }
}

void go(float distanceMm) // Déclacement linéaire prenant en compte l'inversion des moteurs (distance en mm)
{
  leftmotor->move(!LEFT_MOTOR_INVERTED ? DistanceToStep(distanceMm) : -DistanceToStep(distanceMm));
  rightmotor->move(!RIGHT_MOTOR_INVERTED ? DistanceToStep(distanceMm) : -DistanceToStep(distanceMm));
  WaitForMotors();
}

void turn(float angle) // Rotation prenant en compte l'inversion des moteurs (angle en degré)
{
  leftmotor->move(!LEFT_MOTOR_INVERTED ? AngleToStep(angle) : -AngleToStep(angle));
  rightmotor->move(!RIGHT_MOTOR_INVERTED ? -AngleToStep(angle) : AngleToStep(angle));
  WaitForMotors();
}