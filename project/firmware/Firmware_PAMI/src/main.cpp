#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <FastAccelStepper.h>
#include <ESP32Servo.h>
#include <TMCStepper.h>
#include "config.h"

FastAccelStepper leftMotor;
FastAccelStepper rightMotor;
Servo servo1;

// put function declarations here:
int myFunction(int, int);

void setup() {
  servo1.setPeriodHertz(50);       // fréquence habituelle des servos
  servo1.attach(SERVO2, 500, 2400); // GPIO, impulsions min/max en microsecondes
  servo1.write(90);                 // position centrale
  delay(10000);                     // attendre 10 secondes
}

void loop() {
  // put your main code here, to run repeatedly:

  servo1.write(0);   // position 0 degrés
  delay(1000);       // attendre 1 seconde 
  servo1.write(90); // position centrale
  delay(1000);       // attendre 1 seconde 
  servo1.write(180); // position 180 degrés
  delay(1000);       // attendre 1 seconde
  servo1.write(90); // position centrale
  delay(1000);      // attendre 1 seconde
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}