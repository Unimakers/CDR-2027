// Bibliothèques inclus dans PIO + built-in
#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <FastAccelStepper.h>
#include <ESP32Servo.h>
#include <TMCStepper.h>
#include <Wire.h>
#include <Ultrasonic.h>

// Bibliothèques créées
#include <config.h>
#include <monitoring.h>
#include <converting.h>
#include <steppercontrol.h>

Servo sword;
Status status;


void setup()
{ // Initialisation
  Serial.begin(115200);
  delay(1000);
  status.initializing();

  sword.setPeriodHertz(50); // fréquence habituelle des servos
  sword.attach(SWORD_PIN);  // déclaration du servo
  sword.write(90);          // position centrale

  // detector.setTimeout(40000UL); // Timeout détecteur ultrason

  // Initialize the FastAccelStepper engine
  engine.init();
  status.initialized();
  delay(500);

  leftmotor = engine.stepperConnectToPin(LEFT_STEP_PIN, DRIVER_RMT);
  rightmotor = engine.stepperConnectToPin(RIGHT_STEP_PIN, DRIVER_RMT);
  if (leftmotor == nullptr || rightmotor == nullptr)
  {
    status.error();
    while (true)
      delay(1000);
  }
  else
  {
    status.initialized();
  }

  leftmotor->setDirectionPin(LEFT_DIR_PIN);
  rightmotor->setDirectionPin(RIGHT_DIR_PIN);
  status.initialized();

  leftmotor->setEnablePin(ENABLE_PIN);
  rightmotor->setEnablePin(ENABLE_PIN);
  status.initialized();

  leftmotor->setSpeedInHz(STEPPER_SPEED_HZ);
  rightmotor->setSpeedInHz(STEPPER_SPEED_HZ);
  status.initialized();

  leftmotor->setAcceleration(STEPPER_ACCELERATION);
  rightmotor->setAcceleration(STEPPER_ACCELERATION);
  status.initialized();

  leftmotor->enableOutputs();
  rightmotor->enableOutputs();
  status.initialized();
}

void loop()
{
  sword.write(90);

  // while(1)
  // {
  //   sword.write(0);
  //   delay(500);
  //   sword.write(90);
  //   delay(500);
  //   sword.write(180);
  //   delay(500);
  //   sword.write(90);
  //   delay(500);
  // }
  // Serial.println(detector.read());
  // delay(500);  
}