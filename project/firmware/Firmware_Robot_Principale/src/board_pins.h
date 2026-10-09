// Board pinout: exact mirror of the KiCad schematic "carte principale" (ESP32-S3-DevKitC-1, U4).
// Only edit this file when the PCB changes. What each slot is USED for lives in robot_config.h.
// -1 = placeholder, pin not frozen yet; the current schematic GPIO is given in the comment.

#pragma once

#include <stdint.h>

// ---------------------------------------------------------------------------
// Stepper drivers (TMC2209 U5..U10, 24V, STEP/DIR only: UART, DIAG and INDEX not wired)
// Slot index used in robot_config.h = schematic number - 1.
//
//   slot | driver | motor connector | microstep jumpers (MS1 / MS2)
//   -----+--------+-----------------+------------------------------
//     0  |   U5   |       J4        |   JP1  / JP2
//     1  |   U6   |       J7        |   JP3  / JP16
//     2  |   U7   |       J9        |   JP17 / JP18
//     3  |   U8   |       J8        |   JP4  / JP5
//     4  |   U9   |       J10       |   JP6  / JP19
//     5  |   U10  |       J11       |   JP20 / JP21
//
// Note: U7 drives J9 and U8 drives J8 (crossed on the schematic).
// ---------------------------------------------------------------------------
#define STEPPER_SLOT_COUNT 6

#define STEP_1_PIN -1 // schematic: GPIO14
#define DIR_1_PIN -1  // schematic: GPIO17
#define STEP_2_PIN -1 // schematic: GPIO18
#define DIR_2_PIN -1  // schematic: GPIO21
#define STEP_3_PIN -1 // schematic: GPIO35
#define DIR_3_PIN -1  // schematic: GPIO36
#define STEP_4_PIN -1 // schematic: GPIO37
#define DIR_4_PIN -1  // schematic: GPIO38 (onboard RGB LED on DevKitC-1 v1.1)
#define STEP_5_PIN -1 // schematic: GPIO15
#define DIR_5_PIN -1  // schematic: GPIO16
#define STEP_6_PIN -1 // schematic: GPIO47
#define DIR_6_PIN -1  // schematic: GPIO48 (onboard RGB LED on DevKitC-1 v1.0)

// Shared ~EN of the 6 drivers, active LOW.
// schematic: GPIO45. GPIO45 is a strapping pin (flash voltage): a pull-up on this
// line at boot selects 1.8 V flash and the board won't start. Move it before freezing.
#define MOTORS_EN_PIN -1
#define MOTORS_EN_ACTIVE_LOW true

// TMC2209 standalone microstepping, set by the solder jumpers (B = 3V3, A = GND):
//   MS1=0 MS2=0 -> 8   |   MS1=1 MS2=0 -> 2   |   MS1=0 MS2=1 -> 4   |   MS1=1 MS2=1 -> 16

// ---------------------------------------------------------------------------
// Power
// ---------------------------------------------------------------------------
#define EN_5VP_PIN -1 // ON/OFF of the 5VP converter PS1 (servos/pumps), schematic: GPIO3

// ---------------------------------------------------------------------------
// I2C bus (frozen): 3x PCF8574T ICM6..ICM8 + J2 connector, 4.7k pull-ups on board
// ---------------------------------------------------------------------------
#define I2C_SDA_PIN 9
#define I2C_SCL_PIN 8

// PCF8574T address = 0x20 + (A2 A1 A0). Each Ax has a 4.7k pull-down; soldering its
// jumper sets it to 1. With no jumper soldered all three answer on 0x20 -> conflict.
#define PCF_COUNT 3
#define PCF_ICM6_ADDR -1 // jumpers JP7 (A0), JP8 (A1), JP9 (A2)
#define PCF_ICM7_ADDR -1 // jumpers JP10 (A0), JP11 (A1), JP12 (A2)
#define PCF_ICM8_ADDR -1 // jumpers JP13 (A0), JP14 (A1), JP15 (A2)
// ~INT outputs are only pulled up, not wired to the ESP32: inputs must be polled.

// ---------------------------------------------------------------------------
// Actuator connectors "Servo1".."Servo24": 5VP + GND + one signal line on a PCF8574 output.
// ICM6 -> connectors 1..8, ICM7 -> 9..16, ICM8 -> 17..24.
// Inside each group the wiring is interleaved, identical on the 3 PCF:
//   connector in group : 1  2  3  4  5  6  7  8
//   PCF output         : P1 P3 P5 P7 P0 P2 P4 P6
// For connector n (1..24): pcf = (n - 1) / 8, bit = ACTUATOR_PCF_BIT[(n - 1) % 8]
// ---------------------------------------------------------------------------
#define ACTUATOR_CONNECTOR_COUNT 24
#define ACTUATORS_PER_PCF 8
const uint8_t ACTUATOR_PCF_BIT[ACTUATORS_PER_PCF] = {1, 3, 5, 7, 0, 2, 4, 6};

// ---------------------------------------------------------------------------
// TFT screen (J1, SPI, no reset line on the schematic)
// ---------------------------------------------------------------------------
#define TFT_MISO_PIN -1 // "sdo", schematic: GPIO7
#define TFT_MOSI_PIN -1 // "sdi", schematic: GPIO11
#define TFT_SCK_PIN -1  // "sck", schematic: GPIO10
#define TFT_DC_PIN -1   // "dc",  schematic: GPIO12
#define TFT_CS_PIN -1   // "cs",  schematic: GPIO13

// ---------------------------------------------------------------------------
// Inputs (contact to GND, no external pull-up: use INPUT_PULLUP)
// ---------------------------------------------------------------------------
#define TIRETTE_PIN -1  // starting cord, "tir", schematic: GPIO2
#define BUTTON_PIN -1   // push button, "bu", schematic: GPIO4
#define SWITCH_1_PIN -1 // "sw1", schematic: GPIO5
#define SWITCH_2_PIN -1 // "sw2", schematic: GPIO6
#define SWITCH_3_PIN -1 // "sw3", schematic: GPIO1

// ---------------------------------------------------------------------------
// Extension connector J2 ("pins en +"): SDA, SCL + 4 free GPIO
// ---------------------------------------------------------------------------
#define EXT_1_PIN -1 // J2 pin 3, schematic: GPIO42
#define EXT_2_PIN -1 // J2 pin 4, schematic: GPIO41
#define EXT_3_PIN -1 // J2 pin 5, schematic: GPIO40
#define EXT_4_PIN -1 // J2 pin 6, schematic: GPIO46 (strapping pin, keep it free of pull-ups at boot)

// ---------------------------------------------------------------------------
// LIDAR LD06 (J5, frozen): ESP32 UART0, shared with the DevKit USB-UART bridge.
// Logs must therefore go through the native USB port (ARDUINO_USB_CDC_ON_BOOT=1),
// and the LD06 uses Serial0.
// ---------------------------------------------------------------------------
#define LD06_UART_NUM 0
#define LD06_RX_PIN 44 // ESP32 U0RXD <- LD06 TX
#define LD06_TX_PIN 43 // ESP32 U0TXD -> LD06 RX
#define LD06_BAUDRATE 230400
#define LD06_PWM_PIN -1 // motor speed PWM, "pwm", schematic: GPIO39 (lib: 255 = not used)
