#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <FastAccelStepper.h>
#include <ESP32Servo.h>
#include <TMCStepper.h>
#include <Wire.h>
#include "config.h"

FastAccelStepperEngine engine;
FastAccelStepper* leftMotor = nullptr;
FastAccelStepper* rightMotor = nullptr;


// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting setup...");

  // Initialize the FastAccelStepper engine
  engine.init();

  // Connect the left and right motors to their respective step pins
  leftMotor = engine.stepperConnectToPin(LEFT_STEP_PIN);
  rightMotor = engine.stepperConnectToPin(RIGHT_STEP_PIN);

  if (leftMotor == nullptr || rightMotor == nullptr) {
    Serial.println("Error: Could not connect to stepper motors.");
    while (true); // Stop execution if motors cannot be connected
  }

  // Set direction pins for the motors
  leftMotor->setDirectionPin(LEFT_DIR_PIN, true);
  rightMotor->setDirectionPin(RIGHT_DIR_PIN, true);

  // Set enable pins for the motors
  leftMotor->setEnablePin(ENABLE_PIN);
  rightMotor->setEnablePin(ENABLE_PIN);

  
  // Enable the motors
  leftMotor->enableOutputs();
  rightMotor->enableOutputs();

  Serial.println("Setup complete.");
  

}

void loop() {
  // put your main code here, to run repeatedly:
  leftMotor->move(1600);
  rightMotor->move(1600);
  delay(5000);
  leftMotor->move(-1600);
  rightMotor->move(-1600);
  delay(5000);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}