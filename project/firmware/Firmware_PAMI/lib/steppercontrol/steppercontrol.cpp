#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <FastAccelStepper.h>
#include <TMCStepper.h>
#include <Wire.h>

// Bibliothèques créées
#include <config.h>
#include <converting.h>
#include <steppercontrol.h>

FastAccelStepperEngine engine;
FastAccelStepper *leftmotor = nullptr;
FastAccelStepper *rightmotor = nullptr;

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

float setAngle(float start_angleDeg, float wanted_angleDeg)
{
    float angle = wanted_angleDeg - start_angleDeg;
    turn(angle);
    return updateAngle(start_angleDeg, angle);
}




// En cours de travail - Ne pas toucher à moins d'être sûr de ce qu'on fait

// void interrupt()
// {
//   if (leftmotor->isRunning() || rightmotor->isRunning())
//   {
//     int32_t Lrestant = leftmotor->targetPos() - leftmotor->getCurrentPosition();
//     int32_t Rrestant = rightmotor->targetPos() - rightmotor->getCurrentPosition();
//     Serial.printf("%i ; %i\n", Lrestant, Rrestant);
//     Serial.printf("%i ; %i\n", Lrestant - Lrestant, Rrestant - Rrestant);
//     leftmotor->move(!LEFT_MOTOR_INVERTED ? Lrestant : -Lrestant);
//     rightmotor->move(!RIGHT_MOTOR_INVERTED ? -Rrestant : Rrestant);
//   }
// }