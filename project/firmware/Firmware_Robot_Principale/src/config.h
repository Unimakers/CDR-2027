// Configuration file for the robot firmware
// Pin names come from the KiCad schematic "carte principale" (ESP32-S3-DevKitC, U4).
// All pins are placeholders (-1) until assigned; the schematic GPIO is given in the comments.

#pragma once

// Serial debug output
#define DEBUG_SERIAL true

// match duration
#define MATCH_DURATION 100 // in seconds

// ---------------------------------------------------------------------------
// Stepper drivers (TMC2209 U5..U10, 24V)
// ---------------------------------------------------------------------------
#define STEP_1_PIN -1 // schematic: GPIO14
#define DIR_1_PIN -1  // schematic: GPIO17
#define STEP_2_PIN -1 // schematic: GPIO18
#define DIR_2_PIN -1  // schematic: GPIO21
#define STEP_3_PIN -1 // schematic: GPIO35
#define DIR_3_PIN -1  // schematic: GPIO36
#define STEP_4_PIN -1 // schematic: GPIO37
#define DIR_4_PIN -1  // schematic: GPIO38
#define STEP_5_PIN -1 // schematic: GPIO15
#define DIR_5_PIN -1  // schematic: GPIO16
#define STEP_6_PIN -1 // schematic: GPIO47
#define DIR_6_PIN -1  // schematic: GPIO48

// ---------------------------------------------------------------------------
// Power
// ---------------------------------------------------------------------------
#define EN_5VP_PIN -1 // enable of the 5VP (servo) converter, schematic: GPIO3

// ---------------------------------------------------------------------------
// I2C bus (3x PCF8574T I/O expanders ICM6..ICM8, J2 connector)
// ---------------------------------------------------------------------------
#define I2C_SDA_PIN -1 // schematic: GPIO9
#define I2C_SCL_PIN -1 // schematic: GPIO8

// PCF8574 addresses (set by the A0..A2 jumpers)
#define PCF_ICM6_ADDR -1
#define PCF_ICM7_ADDR -1
#define PCF_ICM8_ADDR -1

// ---------------------------------------------------------------------------
// TFT screen (J1, SPI, no reset line on the schematic)
// ---------------------------------------------------------------------------
#define TFT_MISO_PIN -1 // "sdo", schematic: GPIO7
#define TFT_MOSI_PIN -1 // "sdi", schematic: GPIO11
#define TFT_SCK_PIN -1  // "sck", schematic: GPIO10
#define TFT_DC_PIN -1   // "dc",  schematic: GPIO12
#define TFT_CS_PIN -1   // "cs",  schematic: GPIO13

// ---------------------------------------------------------------------------
// Inputs
// ---------------------------------------------------------------------------
#define TIRETTE_PIN -1 // starting cord, "tir", schematic: GPIO2
#define BUTTON_PIN -1  // push button, "bu", schematic: GPIO4
#define SWITCH_1_PIN -1 // "sw1", schematic: GPIO5
#define SWITCH_2_PIN -1 // "sw2", schematic: GPIO6
#define SWITCH_3_PIN -1 // "sw3", schematic: GPIO1

// ---------------------------------------------------------------------------
// LIDAR (J5)
// ---------------------------------------------------------------------------
#define LIDAR_PWM_PIN -1 // motor PWM, "pwm", schematic: GPIO39
