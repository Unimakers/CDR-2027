#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <FastAccelStepper.h>
#include <ESP32Servo.h>
#include <TMCStepper.h>
#include <Wire.h>
#include "config.h"
#include "functions.h"

void setup() { // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  
  sword.setPeriodHertz(50);        // fréquence habituelle des servos
  sword.attach(SWORD_PIN);            // déclaration du servo
  sword.write(90);                 // position centrale
  
  // Initialize the FastAccelStepper engine
  engine.init();
  Serial.println("Motor init");
  delay(500);

  leftmotor = engine.stepperConnectToPin(LEFT_STEP_PIN, DRIVER_RMT);
  rightmotor = engine.stepperConnectToPin(RIGHT_STEP_PIN, DRIVER_RMT);
  if (leftmotor == nullptr || rightmotor == nullptr) {
    Serial.println("Error: Could not connect to motors");
    while (true) {
      delay(1000);
    }
  }
  else
  {
    Serial.println("Motors connected");
  }

  leftmotor->setDirectionPin(LEFT_DIR_PIN);
  rightmotor->setDirectionPin(RIGHT_DIR_PIN);
  Serial.println("DIR Pins connected");

  leftmotor->setEnablePin(ENABLE_PIN);
  rightmotor->setEnablePin(ENABLE_PIN);
  Serial.println("Motors enabled");

  leftmotor->setSpeedInHz(STEPPER_SPEED_HZ);
  rightmotor->setSpeedInHz(STEPPER_SPEED_HZ);
  Serial.println("Motorts speed set");

  leftmotor->setAcceleration(STEPPER_ACCELERATION);
  rightmotor->setAcceleration(STEPPER_ACCELERATION);
  Serial.println("Motorts acceleration set");

  leftmotor->enableOutputs();
  rightmotor->enableOutputs();
  Serial.println("Stepper motors ready");
}

void loop() {
  // put your main code here, to run repeatedly:
  sword.write(0);
  delay(750);
  sword.write(180);
  delay(750);
  sword.write(90);
  delay(750);
  turn(90);
  delay(500);
  turn(-180);
  delay(500);
  turn(90);
  go(200);
  delay(1000);
  go(-200);
  delay(500);
}