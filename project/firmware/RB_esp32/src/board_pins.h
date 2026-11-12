// Board pinout: exact mirror of the KiCad project ECAD-2026 "body v1"
// (Seeed XIAO ESP32-S3 Plus = X1), its two driver boards "leg v1" (2x TMC2209 each,
// plugged on NP1 and NP2) and the head board "head v1" (plugged on NP3).
// Checked against the PCB nets (body v1.kicad_pcb, carte driver.kicad_pcb).
// Only edit this file when a PCB changes. What each slot is USED for lives in robot_config.h.

#pragma once

#include <stdint.h>

// ---------------------------------------------------------------------------
// I2C bus: PCF8574 of the body (drivers), PCF8574 of the head (switches),
// external PCA9685 modules on the 4 I2C connectors IC1..IC4.
// 4.7k pull-ups to 3V3 on the body (R5, R6) and on the head (R7, R8).
// The PCF8574 is a 100 kHz (standard mode) chip: keep the Wire default clock.
// ---------------------------------------------------------------------------
#define I2C_SDA_PIN 5 // D4
#define I2C_SCL_PIN 6 // D5

// ---------------------------------------------------------------------------
// Stepper drivers: 2 driver boards "leg v1", 2 TMC2209 each, 24V, STEP/DIR on the XIAO,
// ~EN and microstepping (MS1/MS2) on the body PCF8574. UART, DIAG, INDEX not wired.
//
//   slot | body conn. | driver board | motor connector | 2026 robot use
//   -----+------------+--------------+-----------------+---------------
//     0  |    NP1     |  U1          |  J3             | back elevator
//     1  |    NP1     |  U2          |  J2             | LEFT wheel
//     2  |    NP2     |  U1          |  J3             | front elevator
//     3  |    NP2     |  U2          |  J2             | RIGHT wheel
//
// WARNING: on the driver board the nets "STEP 1" and "DIR 1" are swapped on U1
// (net "STEP 1" goes to the DIR pad 9, net "DIR 1" to the STEP pad 10). U2 is right.
// The pins below are the REAL function at the TMC2209, so the swap is already corrected:
// slot 0 STEP = net "DIR 1" (GPIO4), slot 2 STEP = net "DIR 3" (GPIO2).
// (The 2026 firmware used the net names, STEP = GPIO3 / GPIO1, for the elevators: if an
// elevator does not move, swap STEP_1/DIR_1 or STEP_3/DIR_3 here. The wheels are on U2,
// not affected, and their pins are the ones validated in 2026.)
// ---------------------------------------------------------------------------
#define STEPPER_SLOT_COUNT 4

#define STEP_1_PIN 4  // net "DIR 1"  (D3)   -> NP1 U1 STEP
#define DIR_1_PIN 3   // net "STEP 1" (D2)   -> NP1 U1 DIR
#define STEP_2_PIN 40 // net "STEP 2" (D13)  -> NP1 U2 STEP
#define DIR_2_PIN 10  // net "DIR 2"  (D16)  -> NP1 U2 DIR
#define STEP_3_PIN 2  // net "DIR 3"  (D1)   -> NP2 U1 STEP
#define DIR_3_PIN 1   // net "STEP 3" (D0)   -> NP2 U1 DIR
#define STEP_4_PIN 38 // net "STEP 4" (D11)  -> NP2 U2 STEP
#define DIR_4_PIN 39  // net "DIR 4"  (D12)  -> NP2 U2 DIR

// Driver board of each slot: both TMC2209 of a board share ~EN, MS1 and MS2.
#define DRIVER_BOARD_COUNT 2
const uint8_t STEPPER_SLOT_BOARD[STEPPER_SLOT_COUNT] = {0, 0, 1, 1}; // slot -> board (0 = NP1, 1 = NP2)

// ---------------------------------------------------------------------------
// PCF8574T ICM1 on the body: ~EN and MS1/MS2 of the 2 driver boards.
// Address = 0x20 + (A2 A1 A0) for a PCF8574T (0x38 + ... would be a PCF8574AT).
// A0..A2 have a pull-down (R1..R3) and a solder jumper to 3V3 (JP1..JP3): 0x20 with no jumper.
// ~INT has a 4.7k pull-up (R4) but is not wired to the XIAO.
//
// At power-on every PCF pin is HIGH (weak pull-up): ~EN = 1 -> drivers disabled,
// MS1 = MS2 = 1 -> 1/16 microstep, until steppersBegin() writes the real values.
//
//   PCF bit | NP pin | driver board signal
//   --------+--------+--------------------
//     P3    | NP1-1  | board 0 ~EN
//     P2    | NP1-2  | board 0 MS1
//     P1    | NP1-3  | (not connected on the driver board)
//     P0    | NP1-4  | board 0 MS2
//     P4    | NP2-1  | board 1 ~EN
//     P5    | NP2-2  | board 1 MS1
//     P6    | NP2-3  | (not connected on the driver board)
//     P7    | NP2-4  | board 1 MS2
// ---------------------------------------------------------------------------
#define PCF_DRIVERS_ADDR 0x20

const uint8_t DRIVER_EN_PCF_BIT[DRIVER_BOARD_COUNT] = {3, 4};
const uint8_t DRIVER_MS1_PCF_BIT[DRIVER_BOARD_COUNT] = {2, 5};
const uint8_t DRIVER_MS2_PCF_BIT[DRIVER_BOARD_COUNT] = {0, 7};
#define DRIVERS_EN_ACTIVE_LOW true

// TMC2209 standalone microstepping (MS1 / MS2 levels):
//   MS1=0 MS2=0 -> 8   |   MS1=1 MS2=0 -> 2   |   MS1=0 MS2=1 -> 4   |   MS1=1 MS2=1 -> 16

// ---------------------------------------------------------------------------
// Power
// ---------------------------------------------------------------------------
#define EN_5VP_PIN 13 // "En5VP" (D17): ON/OFF of the 5VP converter PS1 (servos/pumps)

// ---------------------------------------------------------------------------
// PCA9685 PWM drivers: no longer on the body, external modules on the I2C
// connectors IC1..IC4. Address = 0x40 + jumpers of the module.
// 0x70 is the PCA9685 "all call" address (enabled at power-on): never give it to a chip.
// ---------------------------------------------------------------------------
#define PCA_COUNT 2
#define PCA_CHANNEL_COUNT 16 // outputs per PCA9685

#define PCA_1_ADDR 0x40
#define PCA_2_ADDR 0x41

// ~OE of the PCA modules is not wired to the XIAO (module pull-down = outputs on)
#define PCA_OE_PIN -1

static_assert(PCA_1_ADDR == -1 || PCA_2_ADDR == -1 || PCA_1_ADDR != PCA_2_ADDR,
              "Both PCA9685 have the same I2C address, check the module jumpers");
static_assert(PCA_1_ADDR != 0x70 && PCA_2_ADDR != 0x70,
              "0x70 is the PCA9685 all-call address, choose another one");

// Actuator "connectors" 1..32 = PCA 1 outputs 0..15 then PCA 2 outputs 0..15
// (straight mapping on the external modules, unlike the interleaved old main board).
#define ACTUATOR_CONNECTOR_COUNT (PCA_COUNT * PCA_CHANNEL_COUNT)
const uint8_t ACTUATOR_PCA_CHANNEL[PCA_CHANNEL_COUNT] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

// ---------------------------------------------------------------------------
// Head board (NP3 "IHM + Lidar"): PCF8574T ICM1 with 4 switch connectors J1..J4.
// Same chip and same jumper circuit as the body PCF: with no jumper BOTH PCF answer
// on 0x20 and the bus does not work. Solder JP1 (A0) on the head -> 0x21.
// Switch contact to GND. Quasi-bidirectional pins: write 1 before reading, closed = 0.
// P4..P7 are not connected.
// ---------------------------------------------------------------------------
#define PCF_HEAD_ADDR 0x21

#define SWITCH_COUNT 4
const uint8_t SWITCH_PCF_BIT[SWITCH_COUNT] = {0, 1, 2, 3}; // J1..J4 -> P0..P3

static_assert(PCF_DRIVERS_ADDR != PCF_HEAD_ADDR,
              "Both PCF8574 have the same I2C address: solder JP1 on the head board");
static_assert(PCF_DRIVERS_ADDR != PCA_1_ADDR && PCF_DRIVERS_ADDR != PCA_2_ADDR &&
              PCF_HEAD_ADDR != PCA_1_ADDR && PCF_HEAD_ADDR != PCA_2_ADDR,
              "A PCF8574 shares its I2C address with a PCA9685");

// ---------------------------------------------------------------------------
// Inputs on the XIAO, through the head board (contact to GND: use INPUT_PULLUP)
// ---------------------------------------------------------------------------
#define TIRETTE_PIN 12     // "tir" (D18), head J6
#define TEAM_SWITCH_PIN 11 // "TSW" (D19), head J7

// ---------------------------------------------------------------------------
// WS2812B RGB LEDs on the head (D11 -> D12 chained), 5VL powered
// ---------------------------------------------------------------------------
#define RGB_LED_PIN 7 // "LED" (D8)
#define RGB_LED_COUNT 2

// ---------------------------------------------------------------------------
// LIDAR LD06 on head J5, UART1 on the XIAO hardware UART pins.
// Serial = native USB CDC on the XIAO, so GPIO43/44 are free for the lidar.
// Always pass the pins: Serial1.begin(LD06_BAUDRATE, SERIAL_8N1, LD06_RX_PIN, LD06_TX_PIN),
// and never call LD06::init(), which uses the defaults.
// ---------------------------------------------------------------------------
#define LD06_UART_NUM 1
// The nets are named from the lidar side: the LD06 data arrives on net "TX" (GPIO43).
// Same as Firmware-CDR2026-MainBot (LIDAR_RX_PIN = TX), validated on the 2026 robot.
#define LD06_RX_PIN 43 // net "TX" (D6): XIAO RX <- LD06 TX (head J5 pin 2)
#define LD06_TX_PIN 44 // net "RX" (D7): XIAO TX -> head J5 pin 3
#define LD06_BAUDRATE 230400
#define LD06_PWM_PIN 8 // "PWM" (D9): motor speed PWM, head J5 pin 5

// ---------------------------------------------------------------------------
// XBee (L1: DOUT, DIN, 3V3, GND) on UART2
// ---------------------------------------------------------------------------
#define XBEE_RX_PIN 41 // "DOUT-xbee" (D14): XIAO RX <- XBee DOUT
#define XBEE_TX_PIN 42 // "DIN-xbee"  (D15): XIAO TX -> XBee DIN

// Free: GPIO9 (D10)
