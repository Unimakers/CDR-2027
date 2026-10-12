// Board pinout: exact mirror of the final KiCad schematic "carte principale"
// (project/ecad/carte-principale, ESP32-S3-DevKitC-1 = U4).
// Only edit this file when the PCB changes. What each slot is USED for lives in robot_config.h.

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

#define STEP_1_PIN 4
#define DIR_1_PIN 5
#define STEP_2_PIN 6
#define DIR_2_PIN 15
#define STEP_3_PIN 16
#define DIR_3_PIN 21
#define STEP_4_PIN 35
#define DIR_4_PIN 36
#define STEP_5_PIN 47
#define DIR_5_PIN 48 // also drives the DevKitC-1 v1.0 onboard RGB LED: it flickers with DIR
#define STEP_6_PIN 37
#define DIR_6_PIN 38 // also drives the DevKitC-1 v1.1 onboard RGB LED: it flickers with DIR

// Shared ~EN of the 6 drivers, active LOW. No pull resistor on the board:
// the line floats from power-on until steppersBegin() drives it.
#define MOTORS_EN_PIN 2
#define MOTORS_EN_ACTIVE_LOW true

// TMC2209 standalone microstepping, set by the solder jumpers (B = 3V3, A = GND):
//   MS1=0 MS2=0 -> 8   |   MS1=1 MS2=0 -> 2   |   MS1=0 MS2=1 -> 4   |   MS1=1 MS2=1 -> 16

// ---------------------------------------------------------------------------
// Power
// ---------------------------------------------------------------------------
#define EN_5VP_PIN 1 // ON/OFF of the 5VP converter PS1 (servos/pumps)

// ---------------------------------------------------------------------------
// I2C bus: the 2 PCA9685, 4.7k pull-ups to 3V3 on board (R43, R44)
// GPIO46 is a boot strapping pin: download mode needs it LOW, but the SCL pull-up
// holds it HIGH. If uploading fails with the DevKit plugged on the board, this is why.
// ---------------------------------------------------------------------------
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 46

// ---------------------------------------------------------------------------
// PCA9685 PWM drivers U3 and U11: signal only. The actuators get 5VP + GND from the board.
// ---------------------------------------------------------------------------
#define PCA_COUNT 2
#define PCA_CHANNEL_COUNT 16 // outputs per PCA9685

// Address = 0x40 + (A1 A0). A2..A5 are tied to GND; A0/A1 have a 4.7k pull-down
// and a solder jumper to 3V3. With no jumper soldered both chips answer on 0x40.
//   U3  (PCA 1): A0 = JP8,  A1 = JP9   -> 0x40 with no jumper
//   U11 (PCA 2): A0 = JP10, A1 = JP11  -> 0x41 with JP10 soldered
// 0x70 is the PCA9685 "all call" address (enabled at power-on): never give it to a chip.
#define PCA_1_ADDR 0x40
#define PCA_2_ADDR 0x41

// ~OE of both PCA, active LOW. 10k pull-up (R45): every output is off from
// power-on until actuatorsBegin() drives it LOW.
#define PCA_OE_PIN 9

static_assert(PCA_1_ADDR == -1 || PCA_2_ADDR == -1 || PCA_1_ADDR != PCA_2_ADDR,
              "Both PCA9685 have the same I2C address, check the solder jumpers");
static_assert(PCA_1_ADDR != 0x70 && PCA_2_ADDR != 0x70,
              "0x70 is the PCA9685 all-call address, choose another one");

// ---------------------------------------------------------------------------
// Actuator connectors "Servo1".."Servo32" (GND, 5VP, signal).
// Servo1..16 -> U3 (PCA 1), Servo17..32 -> U11 (PCA 2). Inside each group of 16
// the wiring is interleaved, identically on both chips:
//   connector in group : 1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16
//   PCA output (LEDx)  : 1  3  5  7  0  2  4  6  9  11 13 15 8  10 12 14
// For connector n (1..32): pca = (n - 1) / 16, output = ACTUATOR_PCA_CHANNEL[(n - 1) % 16]
// ---------------------------------------------------------------------------
#define ACTUATOR_CONNECTOR_COUNT (PCA_COUNT * PCA_CHANNEL_COUNT)
const uint8_t ACTUATOR_PCA_CHANNEL[PCA_CHANNEL_COUNT] = {1, 3, 5, 7, 0, 2, 4, 6, 9, 11, 13, 15, 8, 10, 12, 14};

// ---------------------------------------------------------------------------
// TFT screen (J1, SPI, no reset line on the schematic)
// ---------------------------------------------------------------------------
#define TFT_MISO_PIN 7  // "sdo"
#define TFT_MOSI_PIN 11 // "sdi"
#define TFT_SCK_PIN 10  // "sck"
#define TFT_DC_PIN 12   // "dc"
#define TFT_CS_PIN 13   // "cs"

// ---------------------------------------------------------------------------
// Inputs (contact to GND, no external pull-up: use INPUT_PULLUP)
// ---------------------------------------------------------------------------
#define TIRETTE_PIN 39  // starting cord, "tir"
#define BUTTON_PIN 40   // push button, "bu"
#define SWITCH_1_PIN 42 // "sw1"
#define SWITCH_2_PIN 14 // "sw2"

// ---------------------------------------------------------------------------
// LIDAR LD06 (J5) on UART1. UART0 (GPIO43/44) stays free for the Serial logs.
// The Arduino default pins of Serial1 on the S3 are RX 15 / TX 16, i.e. DIR_2 and
// STEP_3 on this board: always pass the pins, Serial1.begin(LD06_BAUDRATE, SERIAL_8N1,
// LD06_RX_PIN, LD06_TX_PIN), and never call LD06::init(), which uses the defaults.
// ---------------------------------------------------------------------------
#define LD06_UART_NUM 1
#define LD06_RX_PIN 17 // ESP32 RX <- LD06 TX (J5 pin 3)
#define LD06_TX_PIN 18 // ESP32 TX -> LD06 RX (J5 pin 2)
#define LD06_BAUDRATE 230400
#define LD06_PWM_PIN 41 // motor speed PWM, J5 pin 5
