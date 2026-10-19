// LD06 LIDAR driver, see ld06.h for the usage and the packet format.

#include "ld06.h"

LD06::LD06(HardwareSerial &serial, uint8_t pwmPin)
  : _lidarSerial(&serial),
    _pin(pwmPin) {}

// Opens the UART on its default pins: do not use on this board (see ld06.h).
void LD06::init() {
  _lidarSerial->begin(230400);
  // PWM input held HIGH: see the LD06 datasheet for the speed control by an external PWM
  if (_pin != 255) {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, HIGH);
  }
}

/* Read lidar packet data with or without checking CRC,
   return : true if a valid package was received (with CRC : only if CRC ok)
*/
bool LD06::readData() {
  return _useCRC ? readDataCRC() : readDataNoCRC();
}

/* Read lidar packet data and check CRC,
   return : true if a valid packet is received
*/
bool LD06::readDataCRC() {
  bool result = false;
  // Small state machine, one byte at a time. _receivedData.index = position in the packet:
  //   0      : waiting for LD06_HEADER (0x54), any other byte is dropped
  //   1      : the next byte must be LD06_VER_SIZE (0x2C), otherwise start over
  //   2..45  : packet content, stored and added to the CRC
  //   46     : last byte = CRC sent by the LD06, compared with the computed one
  while (_lidarSerial->available()) {
    uint8_t current = _lidarSerial->read();
    if (_receivedData.index > 1 || (_receivedData.index == 0 && current == LD06_HEADER) || (_receivedData.index == 1 && current == LD06_VER_SIZE)) {
      _receivedData.packet.bytes[_receivedData.index] = current;
      if (_receivedData.index < LD06_PACKET_SIZE - 1) {
        // CRC-8 computed with a lookup table (ld06crc.h): one table read per byte
        _receivedData.computedCrc = CrcTable[_receivedData.computedCrc ^ current];
        _receivedData.index++;
      } else {
        // Complete packet: decode it only if it was received without error
        if (_receivedData.computedCrc == current) {
          _previousPacket = _receivedData.packet;
          computeData();
          result = true;
        } else {
          _checksumFailCount++;  // CRC error counter
        }
        _receivedData.index = 0;
        _receivedData.computedCrc = 0;
      }
    } else {
      _receivedData.index = 0;
      _receivedData.computedCrc = 0;
    }
  }
  return result;
}

/* Read lidar packet data without checking CRC,
   return : true if a packet is received
*/
bool LD06::readDataNoCRC() {
  bool result = false;
  // Same state machine as readDataCRC(), without the CRC. The packet is decoded as soon as
  // the 46th byte arrives; the CRC byte that follows is then dropped while waiting for
  // the next LD06_HEADER.
  while (_lidarSerial->available()) {
    uint8_t current = _lidarSerial->read();
    if (_receivedData.index > 1 || (_receivedData.index == 0 && current == LD06_HEADER) || (_receivedData.index == 1 && current == LD06_VER_SIZE)) {
      _receivedData.packet.bytes[_receivedData.index] = current;
      _receivedData.index++;
      if (_receivedData.index == LD06_PACKET_SIZE - 1) {
        _previousPacket = _receivedData.packet;
        computeData();
        _receivedData.index = 0;
        result = true;
      }
    } else {
      _receivedData.index = 0;
    }
  }
  return result;
}

/* Read lidar packets and update scan buffers.
   return : true if:
     - in _fullScan mode : a new 360° scan is available
     - otherwise : partial chunk of data is available
*/
bool LD06::readScan() {
  _newScan = false; // set again by computeData() if a scan completes during this call
  bool result = false;
  if (readData()) {
    if (_newScan) {
      result = true;
    }
  }
  return result;
}

/* Converts the 12 measures of the packet just received into DataPoints and adds them
   to the current scan. Detects the end of a turn: the angle goes back below the previous
   one (passing 360 -> 0). A turn is accepted as a complete scan only if it covered more
   than 340 degrees, so the partial first turn after startup is never published.
   The static variables keep their value between packets (only one LD06 per program).
*/
void LD06::computeData() {
  static bool  isInit         = false;
  static float lastPhysAngle  = 0.0f;  // Previous point LD06 angle data CW
  static float startPhysAngle = 0.0f;  // LD06 starting angle data CW

  float angleStep = getAngleStep();
  if (angleStep > LD06_ANGLE_STEP_MAX || angleStep <= 0.0f) {
    // should not be possible
    isInit = false;
    return;
  }

  int8_t reverse = (_upsideDown ? -1 : 1);  // not used

  float fsa = _receivedData.packet.startAngle / 100.0f;  // first measure angle, in degrees

  DataPoint data;

  for (uint16_t i = 0; i < LD06_PTS_PER_PACKETS; i++) {

    // The 12 measures are spread evenly between startAngle and endAngle
    float physAngle = fsa + (i + 0.5f) * angleStep;

    while (physAngle >= 360.0f) physAngle -= 360.0f;
    while (physAngle <   0.0f) physAngle += 360.0f;

    // Angle smaller than the previous one: the lidar just passed 0 degrees, new turn
    if (physAngle < lastPhysAngle) {
      if (!isInit) {
        isInit = true;
      } else {
        if (lastPhysAngle - startPhysAngle > 340.0f) {
          _newScan = true;
          if (_fullScan) {
            swapBuffers();
          }
        }
      }
      startPhysAngle = physAngle;
    }
    lastPhysAngle = physAngle;


    float angle;
    if (_upsideDown) {
      angle = physAngle;                 // Upside down turn CW to CCW => Nothing to change
    } else {
      angle = 360.0f - physAngle;        // Convert LD06 angle data CW to mathematical CCW convension
      if (angle >= 360.0f) angle -= 360.0f;
    }

    // Normalize [0 360°]
    if (angle < 0.0f)     angle += 360.0f;
    if (angle >= 360.0f)  angle -= 360.0f;

    _angles[i] = angle;

    // Points beyond LD06_MAX_PTS_SCAN are dropped (slow rotation = more points per turn)
    if (isInit && _currentScan->index < LD06_MAX_PTS_SCAN) {
      data.angle     = angle;                                   // Mathematical angle CCW
      data.distance  = _receivedData.packet.measures[i].distance;
      data.intensity = _receivedData.packet.measures[i].intensity;

      if (!_useFiltering || filter(data)) {
#ifdef LD06_COMPUTE_XY
        // Point in the table frame:
        //   lidar position = robot position + lidar offset rotated by the robot angle
        //   point          = lidar position + distance in the direction
        //                    (point angle + robot angle + lidar angular offset)
        // NOTE: y uses "- distance * sin" while x/y of the offset use the usual rotation:
        // check the sign of y with a real scan before using it for localisation.
        float angRad = (data.angle + _angularPosition + _angularOffset) * PI / 180.0f;
        float cosPos = cos(_angularPosition * PI / 180.0f);
        float sinPos = sin(_angularPosition * PI / 180.0f);

        data.x = _xPosition
                 + _xOffset * cosPos - _yOffset * sinPos
                 + data.distance * cos(angRad);

        data.y = _yPosition
                 + _xOffset * sinPos + _yOffset * cosPos
                 - data.distance * sin(angRad);
#endif
        _currentScan->points[_currentScan->index++] = data;
      }
    }
  }

  // Not in full scan mode: every packet triggers a swap (one packet = one "scan")
  if (!_fullScan) {
    swapBuffers();
    _newScan = true;
  }
}

// Print full scan using csv format
void LD06::printScanCSV(Stream &serialport) {
  static bool init = false;
  if (!init) {
    serialport.println(F("N,Angle(°),Distance(mm),Intensity,x(mm),y(mm)"));
    init = true;
  }
  if (_previousScan->index) {
    for (uint16_t i = 0; i < _previousScan->index; i++) {
      serialport.print(i);
      serialport.print(",");
      serialport.print(_previousScan->points[i].angle, 2);
      serialport.print(",");
      serialport.print(_previousScan->points[i].distance);
      serialport.print(",");
      serialport.print(_previousScan->points[i].intensity);
#ifdef LD06_COMPUTE_XY
      serialport.print(",");
      serialport.print(_previousScan->points[i].x);
      serialport.print(",");
      serialport.print(_previousScan->points[i].y);
#endif
      serialport.println();
    }
    serialport.println();
  }
}

// Print full scan using teleplot format (check :https://teleplot.fr/)
#ifdef LD06_COMPUTE_XY
void LD06::printScanTeleplot(Stream &serialport) {
  if (_previousScan->index) {
    serialport.print(F(">lidar:"));
    for (uint16_t i = 0; i < _previousScan->index; i++) {
      serialport.print(_previousScan->points[i].x);
      serialport.print(":");
      serialport.print(_previousScan->points[i].y);
      serialport.print(";");
    }
    serialport.println(F("|xy"));
  }
}
#endif

// Settings
void LD06::enableCRC() {
  _useCRC = true;
}

void LD06::disableCRC() {
  _useCRC = false;
}

void LD06::enableFullScan() {   // readScan will return true only when a new 360° scan is available
  _fullScan = true;
}
void LD06::disableFullScan() {  // readScan will return true for each data chunk
  _fullScan = false;
}

void LD06::enableFiltering() {
  _useFiltering = true;
}

void LD06::disableFiltering() {
  _useFiltering = false;
}

void LD06::setIntensityThreshold(uint8_t threshold) {
  _threshold = threshold;
}

void LD06::setMaxDistance(uint16_t maxDist) {
  _maxDist = maxDist;
}

void LD06::setMinDistance(uint16_t minDist) {
  _minDist = minDist;
}

void LD06::setDistanceRange(uint16_t minDist, uint16_t maxDist) {
  _minDist = minDist;
  _maxDist = maxDist;
}

// Brings an angle back into 0..360 degrees (360 itself is kept as 360)
int16_t LD06::rescaleAngle(int16_t angle) {
  if (angle > 360)
    angle %= 360;
  else
    while (angle < 0)
      angle += 360;
  return angle;
}

void LD06::setMaxAngle(int16_t maxAngle) {
  _maxAngle = rescaleAngle(maxAngle);
}

// Unlike setMaxAngle() and setAngleRange(), the value is not passed through rescaleAngle():
// give an angle already in 0..360.
void LD06::setMinAngle(int16_t minAngle) {
  _minAngle = minAngle;
}

void LD06::setAngleRange(int16_t minAngle, int16_t maxAngle) {
  _minAngle = rescaleAngle(minAngle);
  _maxAngle = rescaleAngle(maxAngle);
}

void LD06::setUpsideDown(bool upsideDown) {
  _upsideDown = upsideDown;
}

#ifdef LD06_COMPUTE_XY
void LD06::setOffsetPosition(int16_t xPos = 0, int16_t yPos = 0, float anglePos = 0) {
  _xOffset = xPos;
  _yOffset = yPos;
  _angularOffset = anglePos;
}
#endif

// Double buffer: the scan being filled becomes the readable one, the other buffer is
// emptied and receives the next points. The first call only empties _scanA and sets
// _previousScan to the (empty) _scanB: the real alternation starts at the second call.
void LD06::swapBuffers() {
  if (_currentBuffer) {
    _currentScan = &_scanB;
    _previousScan = &_scanA;
  } else {
    _currentScan = &_scanA;
    _previousScan = &_scanB;
  }
  _currentBuffer = !_currentBuffer;
  _currentScan->index = 0;
}