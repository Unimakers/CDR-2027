// Current main = copy of breadboard/test9_square_lidar.cpp (square + LiDAR, started by the tirette).
// To run another test, copy its file from breadboard/ over this one.
//
// TEST 9 C++: drive in a square (test 7) with the LiDAR LD06 (test 8): stops in front of
// an obstacle, tirette start
// RESULTS : N/A
//
// Wiring: test 7 + test 8. Left wheel NP1 U2 = slot 1, right wheel NP2 U2 = slot 3, 24V ON,
// both PCF8574 answering (0x20 body, 0x21 head), LD06 on head J5, tirette on head J6.
// Sequence:
//   1. boot: if the tirette is not in place, insert it (motors enabled, robot holds still)
//   2. pull the tirette -> the robot drives the square forever (forward SQUARE_SIDE_MM, turn
//      90 deg, x4, pause, repeat) and the scans are streamed in teleplot format
//   3. put the tirette back -> the wheels decelerate to a stop, streaming stops.
//      Pull it again -> the square restarts from its first side.
//
// Obstacle stop (360 deg, all around the robot):
//   - the LiDAR filter keeps the points between LD06_MIN_DISTANCE and LD06_MAX_DISTANCE
//     (robot_config.h): MIN hides the robot's own chassis and the noise, MAX the far points
//   - if at least OBSTACLE_MIN_POINTS of these points are closer than LD06_STOP_DISTANCE,
//     the wheels decelerate to a stop. STOP must include the braking distance.
//   - when no obstacle has been seen for OBSTACLE_CLEAR_MS, the interrupted move resumes
//     towards its original target: the square keeps its shape
//   - no LiDAR scan for LIDAR_TIMEOUT_MS (connector, power) = stop too, same as an obstacle
//
// The square is a non-blocking state machine (unlike test 7): loop() must come back often
// to empty the LiDAR UART buffer. FastAccelStepper generates the steps in hardware, so
// printing a scan (several ms) does not disturb the wheels.
//
// Tuning: same as test 7 (WHEEL_DIAMETER_MM, WHEEL_BASE_MM, TURN_DIRECTION).
// Serial commands (115200): p = pause/resume (after the current move),
// d = disable motors (free wheels), e = enable.
// To run it: copy this file over src/main.cpp.

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "inputs.h"
#include "ld06.h"
#include "steppers.h"

#define TIRETTE_DEBOUNCE_MS 50

#define WHEEL_DIAMETER_MM 68.0f // 2026 robot wheels
#define WHEEL_BASE_MM 250.0f    // TODO: measure (contact point to contact point)
#define SQUARE_SIDE_MM 500.0f
#define TURN_DIRECTION 1        // 1 = left turns (counter-clockwise square), -1 = right turns
#define SETTLE_MS 300           // pause after each move
#define SQUARE_PAUSE_MS 2000    // pause after each full square
#define SQUARE_STEPS 8          // 4 x (forward + turn)

#define LD06_STOP_DISTANCE 300  // mm, obstacle closer than this = stop (TODO: tune, can move to robot_config.h)
#define OBSTACLE_MIN_POINTS 3   // points below LD06_STOP_DISTANCE needed in one scan (1 = stops on noise)
#define OBSTACLE_CLEAR_MS 500   // obstacle gone for this long -> the robot restarts
#define LIDAR_TIMEOUT_MS 500    // no scan for this long -> stop (10 scans/s normally)

// The library stores the distances unsigned: -1 would become 65535 and filter every point.
// MIN must be > 0: the LD06 reports 0 mm for an invalid measure.
static_assert(LD06_MIN_DISTANCE > 0, "set LD06_MIN_DISTANCE (mm, > 0) in robot_config.h");
static_assert(LD06_MAX_DISTANCE > LD06_MIN_DISTANCE, "set LD06_MAX_DISTANCE (mm, > MIN) in robot_config.h");
static_assert(LD06_STOP_DISTANCE > LD06_MIN_DISTANCE && LD06_STOP_DISTANCE <= LD06_MAX_DISTANCE,
              "LD06_STOP_DISTANCE must be between LD06_MIN_DISTANCE and LD06_MAX_DISTANCE");

LD06 ld06(Serial1);

static FastAccelStepper* leftWheel = nullptr;
static FastAccelStepper* rightWheel = nullptr;
static bool paused = false;
static bool motorsOn = false;
static bool running = false; // tirette pulled: square + lidar stream

enum class Phase { WAIT, MOVING, SETTLE, SQUARE_DONE };
static Phase phase = Phase::WAIT;
static int squareStep = 0; // even = forward, odd = turn
static uint32_t phaseAt = 0;
static long targetLeft = 0, targetRight = 0; // absolute targets of the current move
static bool moveInterrupted = false;         // current move stopped by an obstacle

// Obstacle detection, updated on every new scan
static bool obstacle = false;       // an obstacle is (or was, < OBSTACLE_CLEAR_MS ago) too close
static uint32_t obstacleSeenAt = 0;
static uint32_t lastScanAt = 0;
static uint16_t nearestMm = 0;      // nearest filtered point of the last scan, 0 = none

// ---------------------------------------------------------------------------
// Tirette
// ---------------------------------------------------------------------------

// Tirette state, only changes after TIRETTE_DEBOUNCE_MS of stable reading
static bool tiretteDebounced() {
    static bool stable = tiretteIsInserted();
    static bool last = stable;
    static uint32_t changedAt = 0;
    bool now = tiretteIsInserted();
    if (now != last) {
        last = now;
        changedAt = millis();
    } else if (now != stable && millis() - changedAt >= TIRETTE_DEBOUNCE_MS) {
        stable = now;
    }
    return stable;
}

// ---------------------------------------------------------------------------
// Wheels (same geometry as test 7)
// ---------------------------------------------------------------------------

// microsteps per mm of wheel travel (same steps/rev on both wheels)
static float stepsPerMm() {
    return stepperSlotStepsPerRev(WHEEL_LEFT_SLOT) / (PI * WHEEL_DIAMETER_MM);
}

static long distanceToSteps(float mm) {
    return lroundf(mm * stepsPerMm());
}

// On-the-spot rotation: each wheel travels an arc of radius WHEEL_BASE_MM / 2
static long angleToSteps(float deg) {
    return distanceToSteps(deg * PI / 180.0f * (WHEEL_BASE_MM / 2.0f));
}

static void setMotors(bool on) {
    stepperMotorsEnable(on);
    motorsOn = on;
}

static bool wheelsRunning() {
    return (leftWheel && leftWheel->isRunning()) || (rightWheel && rightWheel->isRunning());
}

// Relative move of both wheels; remembers the absolute targets to resume after an obstacle
static void moveWheels(const char* label, long leftSteps, long rightSteps) {
    Serial.printf("[MOVE] %s: L %+ld  R %+ld usteps\n", label, leftSteps, rightSteps);
    if (leftWheel) {
        targetLeft = leftWheel->getCurrentPosition() + leftSteps;
        leftWheel->moveTo(targetLeft);
    }
    if (rightWheel) {
        targetRight = rightWheel->getCurrentPosition() + rightSteps;
        rightWheel->moveTo(targetRight);
    }
}

// ---------------------------------------------------------------------------
// Obstacle detection
// ---------------------------------------------------------------------------

// Called on every new scan: the scan only holds points between MIN and MAX (filter)
static void checkObstacle() {
    lastScanAt = millis();
    uint16_t n = ld06.getNbPointsInScan();
    uint16_t close = 0;
    nearestMm = 0;
    for (uint16_t i = 0; i < n; i++) {
        uint16_t d = ld06.getPoints(i)->distance;
        if (nearestMm == 0 || d < nearestMm) nearestMm = d;
        if (d < LD06_STOP_DISTANCE) close++;
    }
    if (close >= OBSTACLE_MIN_POINTS) {
        if (!obstacle) {
            Serial.printf("[OBST] obstacle at %u mm (%u points < %u mm) -> stop\n", nearestMm, close,
                          (unsigned)LD06_STOP_DISTANCE);
        }
        obstacle = true;
        obstacleSeenAt = millis();
    }
}

// true = the robot must not move: obstacle too close, or no LiDAR data
static bool robotBlocked() {
    if (obstacle && millis() - obstacleSeenAt >= OBSTACLE_CLEAR_MS) {
        obstacle = false;
        Serial.println("[OBST] path clear");
    }
    static bool lidarLost = false;
    bool lost = millis() - lastScanAt >= LIDAR_TIMEOUT_MS;
    if (lost != lidarLost) {
        lidarLost = lost;
        Serial.println(lost ? "[OBST] no LiDAR scan -> stop (check the J5 connector)" : "[OBST] LiDAR back");
    }
    return obstacle || lost;
}

// Starts move number squareStep of the square
static void startSquareStep() {
    if (squareStep % 2 == 0) {
        long s = distanceToSteps(SQUARE_SIDE_MM);
        moveWheels("forward", s, s);
    } else {
        // deg > 0 = counter-clockwise (left), deg < 0 = clockwise (right)
        long s = angleToSteps(TURN_DIRECTION * 90.0f);
        moveWheels("turn", -s, s);
    }
    phase = Phase::MOVING;
}

// Stops the square: decelerates (or cuts at once if hard = true), back to WAIT
static void stopSquare(bool hard) {
    if (hard) {
        if (leftWheel) leftWheel->forceStop();
        if (rightWheel) rightWheel->forceStop();
    } else {
        if (leftWheel) leftWheel->stopMove();
        if (rightWheel) rightWheel->stopMove();
    }
    phase = Phase::WAIT;
    moveInterrupted = false;
}

// Square state machine, called on every loop() pass, never blocks
static void updateSquare() {
    bool blocked = robotBlocked();
    bool canMove = running && !paused && motorsOn && !blocked;
    switch (phase) {
    case Phase::WAIT:
        // also wait for the end of a deceleration (tirette put back then pulled quickly)
        if (canMove && !wheelsRunning()) {
            squareStep = 0;
            startSquareStep();
        }
        break;
    case Phase::MOVING:
        if (blocked) {
            if (!moveInterrupted) {
                if (leftWheel) leftWheel->stopMove(); // decelerate, the targets are kept
                if (rightWheel) rightWheel->stopMove();
                moveInterrupted = true;
            }
        } else if (moveInterrupted) {
            // obstacle gone: once fully stopped, finish the move towards the same targets
            if (!wheelsRunning()) {
                Serial.println("[MOVE] resume");
                if (leftWheel) leftWheel->moveTo(targetLeft);
                if (rightWheel) rightWheel->moveTo(targetRight);
                moveInterrupted = false;
            }
        } else if (!wheelsRunning()) {
            phase = Phase::SETTLE;
            phaseAt = millis();
        }
        break;
    case Phase::SETTLE:
        if (canMove && millis() - phaseAt >= SETTLE_MS) {
            squareStep++;
            if (squareStep >= SQUARE_STEPS) {
                Serial.printf("[TEST] square done (uptime %lu ms)\n", millis());
                phase = Phase::SQUARE_DONE;
                phaseAt = millis();
            } else {
                startSquareStep();
            }
        }
        break;
    case Phase::SQUARE_DONE:
        if (canMove && millis() - phaseAt >= SQUARE_PAUSE_MS) {
            squareStep = 0;
            startSquareStep();
        }
        break;
    }
}

static void handleSerial() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == 'p') {
            paused = !paused;
            Serial.println(paused ? "[TEST] PAUSED after this move (send p to resume)" : "[TEST] resumed");
        } else if (c == 'd') {
            stopSquare(true);
            setMotors(false);
            Serial.println("[TEST] motors disabled (send e to enable)");
        } else if (c == 'e') {
            setMotors(true);
            Serial.println("[TEST] motors enabled");
        }
    }
}

// ---------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    unsigned long t0 = millis();
    while (!Serial && millis() - t0 < 5000) delay(10);
    delay(500);
    Serial.println("\n[TEST] Test 9: square + LiDAR LD06 + tirette");
    Serial.println("[TEST] commands: p = pause/resume, d = disable motors, e = enable");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    if (!inputsBegin()) {
        Serial.println("[TEST] head PCF8574 not answering (switches only, the tirette still works)");
    }

    // Do NOT call ld06.init(): it opens Serial1 on the default pins, wrong on this board.
    // Big RX buffer: printing a scan to USB takes several ms, the 256-byte default
    // would overflow meanwhile (CRC errors, lost packets)
    Serial1.setRxBufferSize(4096);
    Serial1.begin(LD06_BAUDRATE, SERIAL_8N1, LD06_RX_PIN, LD06_TX_PIN);
    // Distance filter only: the angle / intensity values of robot_config.h are not set yet
    ld06.setDistanceRange(LD06_MIN_DISTANCE, LD06_MAX_DISTANCE);
    ld06.enableFiltering();
    // PWM input of the LD06 held LOW = internal speed control (~10 turns/s)
    pinMode(LD06_PWM_PIN, OUTPUT);
    digitalWrite(LD06_PWM_PIN, LOW);

    steppersBegin();
    leftWheel = connectStepperSlot(WHEEL_LEFT_SLOT);
    rightWheel = connectStepperSlot(WHEEL_RIGHT_SLOT);
    if (!leftWheel || !rightWheel) {
        Serial.println("[TEST] ERROR: a wheel is not usable, check robot_config.h");
    }
    Serial.printf("[TEST] %.2f usteps/mm, side %.0f mm = %ld usteps, 90 deg = %ld usteps per wheel\n",
                  stepsPerMm(), SQUARE_SIDE_MM, distanceToSteps(SQUARE_SIDE_MM), angleToSteps(90));
    Serial.printf("[TEST] lidar filter %u..%u mm, stop below %u mm\n", (unsigned)LD06_MIN_DISTANCE,
                  (unsigned)LD06_MAX_DISTANCE, (unsigned)LD06_STOP_DISTANCE);
    setMotors(true);

    if (!tiretteIsInserted()) {
        Serial.println("[TEST] insert the tirette...");
        while (!tiretteDebounced()) {
            if (ld06.readScan()) checkObstacle(); // keep the UART buffer empty while waiting
            delay(1);
        }
    }
    Serial.println("[TEST] tirette in place, pull it to start the square + lidar stream");
}

void loop() {
    // Read the UART on every pass: the buffer must never overflow
    static uint32_t bytesRx = 0, scans = 0, lastPoints = 0;
    bytesRx += Serial1.available();
    bool newScan = ld06.readScan();
    if (newScan) {
        scans++;
        lastPoints = ld06.getNbPointsInScan();
        checkObstacle();
    }

    // Diagnostic every second (a plain line = shown in the teleplot console):
    //   (bytes/s is approximate: it reads ~12500 when the lidar really sends ~17 kB/s)
    //   bytes/s ~0      -> nothing on GPIO43: wiring of J5, LD06 power (connector contact!)
    //   bytes/s >10000 but scans/s 0 -> data received but not decoded (crc errors?)
    //   scans/s ~10, points ~450    -> lidar OK (fewer points with the MIN/MAX filter)
    //   nearest = closest filtered point, 0 = nothing between MIN and MAX
    static uint32_t lastDiag = 0;
    if (millis() - lastDiag >= 1000) {
        lastDiag = millis();
        Serial.printf("[DIAG] tirette=%s run=%s step=%d/%d obst=%s nearest=%u mm bytes/s=%lu scans/s=%lu points=%lu speed=%u deg/s crcFail=%u\n",
                      tiretteDebounced() ? "in" : "out", running ? "on" : "off", squareStep, SQUARE_STEPS,
                      obstacle ? "YES" : "no", nearestMm,
                      (unsigned long)bytesRx, (unsigned long)scans, (unsigned long)lastPoints,
                      ld06.getSpeed(), ld06.getChecksumFailCount());
        bytesRx = 0;
        scans = 0;
    }

    handleSerial();

    bool inserted = tiretteDebounced();
    if (running && inserted) {
        running = false;
        stopSquare(false);
        Serial.println("[TEST] tirette inserted -> robot stopped, stream stopped, pull it to restart");
    } else if (!running && !inserted) {
        running = true;
        Serial.println("[TEST] tirette pulled -> square + lidar stream started");
    }

    updateSquare();

    if (running && newScan) {
        ld06.printScanTeleplot(Serial);
    }
}
