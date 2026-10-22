#pragma once

// PIN moteurs pas-à-pas
#define RIGHT_DIR_PIN 0
#define RIGHT_STEP_PIN 1
#define LEFT_DIR_PIN 2
#define LEFT_STEP_PIN 3

// Si un moteur tourne dans le sens inverse, mettre sur "true"
#define LEFT_MOTOR_INVERTED   true // synchronisation des roues
#define RIGHT_MOTOR_INVERTED  false

#define WHEEL_DIAMETER_MM    62.4f   // Diamètre exterieur des roues (en mm)
#define WHEEL_BASE_MM        96.6f  // Distance entre les deux points de contact des roues (en mm)
#define MOTOR_STEPS_PER_REV  200     // Pas par tour de moteur (1.8 deg/pas motor)
#define MICROSTEPS           16      // Microstepping réglé sur le TMC2209
#define GEAR_RATIO           1.0f    // 1.0 = pas de réduction

#define TMC2209_MS1_VALUE  HIGH   // HIGH for 1/16
#define TMC2209_MS2_VALUE  HIGH   // HIGH for 1/16

// PIN des servomoteurs
#define SERVO1 4
#define SERVO2 5


// PIN activation élec
#define ENABLE_PIN 8

// PIN tirette
#define TIRETTE_PIN 10

#define SWITCH_PIN 9

// PIN du télémètre à ultrason
#define TRIGER_PIN 7
#define ECHO_PIN 6
#define S_ITERATIONS 5

// intervalles
#define SONAR_PING_INTERVAL 33