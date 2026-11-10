#pragma once

// PIN moteurs pas-à-pas
#define RIGHT_STEP_PIN 1
#define LEFT_STEP_PIN 4

#define RIGHT_DIR_PIN 2
#define LEFT_DIR_PIN 3

// PIN activation des moteurs pas-à-pas
#define ENABLE_PIN 7

// Si un moteur tourne dans le sens inverse, mettre sur "true"
#define LEFT_MOTOR_INVERTED   true 
#define RIGHT_MOTOR_INVERTED  false

#define  STEPPER_SPEED_HZ 5000
#define STEPPER_ACCELERATION 1000

// Pour les déplacements linéaires
#define WHEEL_DIAMETER_MM    62.4f   // Diamètre exterieur des roues (en mm)
#define MICROSTEPS           16      // Microstepping réglé sur le TMC2209
#define GEAR_RATIO           1.0f    // 1.0 = pas de réduction
#define MOTOR_STEPS_PER_REV  200     // Pas par tour de moteur (1.8 deg/pas motor)

const float wheelCircumferenceMm = PI * WHEEL_DIAMETER_MM; // calcul de la circonférence des roues
const float stepsPerWheelRevolution = MOTOR_STEPS_PER_REV * MICROSTEPS * GEAR_RATIO; // calcul du nombre de step par tour de stepper

// Pour les rotations et virages
#define WHEEL_BASE_MM 96.6f  // Distance entre les deux points de contact des roues (en mm)

#define TMC2209_MS1_VALUE  HIGH   // HIGH for 1/16
#define TMC2209_MS2_VALUE  HIGH   // HIGH for 1/16

// PIN des servomoteurs
#define CATAPULT_PIN 5
#define SWORD_PIN 6

// PIN tirette
#define TIRETTE_PIN 9

#define SWITCH_PIN 8

// PIN du télémètre à ultrason
//#define TRIGER_PIN 44
//#define ECHO_PIN 43

// intervalles
#define SONAR_PING_INTERVAL 33