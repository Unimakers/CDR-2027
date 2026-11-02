// Driver for the LDRobot LD06 LIDAR (connector J5, see board_pins.h).
//
// The LD06 spins at ~10 turns/s and streams, without being asked, 47-byte packets of
// 12 measures on its TX line (230400 baud, 8N1): about 4500 points per second.
// This class reassembles the packets, converts each measure to an angle (and x/y if
// LD06_COMPUTE_XY is defined) and stores them in a scan buffer. Two buffers are used
// in turn: one is filled by the incoming packets while the other one holds the last
// complete scan, which is the one read by getPoints() / printScan...().
//
// Typical use (declare the object as a global: its 2 buffers take ~38 kB of RAM with
// LD06_MAX_PTS_SCAN = 1200, far too much for the stack):
//   LD06 lidar(Serial1, LD06_PWM_PIN);
//   setup(): Serial1.begin(LD06_BAUDRATE, SERIAL_8N1, LD06_RX_PIN, LD06_TX_PIN);
//            (do NOT call lidar.init(): it opens Serial1 on the default pins, wrong on this board)
//   loop():  if (lidar.readScan()) {          // call it often: the 256-byte UART buffer
//                                             // fills up in ~11 ms at 230400 baud
//              for (uint16_t i = 0; i < lidar.getNbPointsInScan(); i++) {
//                DataPoint* p = lidar.getPoints(i); // p->angle, p->distance, p->x, p->y...
//              }
//            }
//
// The LD06 itself turns clockwise; computeData() converts its angles to degrees
// counter-clockwise (mathematical convention). setUpsideDown(true) for a lidar mounted
// upside down. Check the 0° direction on the real robot with printScanTeleplot().

#ifndef ld06_h
#define ld06_h

#include "Arduino.h"
#include "ld06crc.h"


// Configuration
#define LD06_COMPUTE_XY         // Comment this if you don't want to use x and y computation
#define LD06_MAX_PTS_SCAN 1200  // 480 is a correct value for typical 10 Hz rotation. 1200 is when using pwm pin to reduce lidar rotation frequency to 4Hz... You can reduce it to save memory space

// Packets size
const uint8_t LD06_PACKET_SIZE = 47;  // Note: 1(Start)+1(Datalen)+2(Speed)+2(SAngle)+36(DataByte)+2(EAngle)+2(TimeStamp)+1(CRC)
const uint8_t LD06_PTS_PER_PACKETS = 12;  // measures per packet
const uint8_t LD06_MEASURE_SIZE = 3;      // bytes per measure: distance (2) + intensity (1)

// Headers: the first 2 bytes of every packet, used to find the start of a packet in the stream
const uint8_t LD06_HEADER = 0x54;
const uint8_t LD06_VER_SIZE = 0x2c;  // packet type 1 (3 high bits) + 12 measures (5 low bits)

// Parameters
// Max angle between 2 measures of a packet, in degrees. A larger value means a corrupted
// packet (normal value: ~0.8 deg at 10 turns/s).
const uint8_t LD06_ANGLE_STEP_MAX = 5;

// The structures below map the raw bytes of a packet: the LD06 sends little-endian values,
// like the ESP32, so the fields can be read directly once all the bytes are in place.
// __attribute__((packed)) removes the padding the compiler would otherwise add.

// Data structures
struct LD06Measure {
  union {
    struct {
      uint16_t distance;  // Unit is mm
      uint8_t intensity;  // Unsigned value 0-255 (signal strength, low on dark or far objects)
    } __attribute__((packed));
    uint8_t bytes[LD06_MEASURE_SIZE];
  };
} __attribute__((packed));

struct LD06Packet {
  union {
    struct {
      uint8_t header;                              // LD06_HEADER
      uint8_t version_size;                        // LD06_VER_SIZE
      uint16_t lidarSpeed;                         // Degrees ° / second
      uint16_t startAngle;                         // Unit is 0.01 Degree°
      LD06Measure measures[LD06_PTS_PER_PACKETS];  // 12 distances data distance + intensity
      uint16_t endAngle;                           // Unit is 0.01 Degree°
      uint16_t timeStamp;                          // Unit is ms (wraps around at 30000)
      uint8_t crc;                                 // Checksum
    } __attribute__((packed));
    uint8_t bytes[LD06_PACKET_SIZE];
  };
} __attribute__((packed));

// Packet being received, byte after byte
struct LD06PacketHandler {
  LD06Packet packet;
  uint8_t computedCrc;  // CRC of the bytes received so far
  uint8_t index;        // next byte to fill, 0 = waiting for LD06_HEADER
};

// One measure, ready to use
struct DataPoint {
  uint16_t distance = 0;  // mm
  float angle = 0.0f;     // degrees
#ifdef LD06_COMPUTE_XY
  int16_t x = 0;  // mm
  int16_t y = 0;  // mm
#endif
  uint8_t intensity = 0;  // 0-255
};

// One scan buffer: index = number of points stored
struct DataPointHandler {
  DataPoint points[LD06_MAX_PTS_SCAN];
  uint16_t index;
};

class LD06 {
public:
  // serial: UART connected to the LD06 TX. pin: GPIO of the LD06 PWM input, 255 = not used.
  LD06(HardwareSerial &serial, uint8_t pin = 255);
  // Opens the UART on its DEFAULT pins and sets the PWM pin HIGH.
  // Not usable on this board: open Serial1 yourself with the pins of board_pins.h.
  void init();
  // Reads every byte waiting in the UART. Returns true when a new scan is available
  // (full scan mode) or when a new packet was decoded (otherwise). Never blocks.
  bool readScan();

  // Print Data over Serial
  void printScanCSV(Stream &serialport);  // Print full scan using csv format
#ifdef LD06_COMPUTE_XY
  void printScanTeleplot(Stream &serialport);  // Print full scan using teleplot format (check :https://teleplot.fr/)
#endif

  // Settings
  void enableCRC();         // Enable CRC checking
  void disableCRC();        // Disable CRC checking
  void enableFullScan();    // Enable full scan mode, readScan will return true only when a new 360° scan is available
  void disableFullScan();   // Disable full scan mode, readScan will return true for each data chunks
  void enableFiltering();   // Enable filtering, lidar points available are only the ones that pass the filter
  void disableFiltering();  // Disable filtering, all lidar points are available

  // Filtering parameters (only applied after enableFiltering(), to the points of the next packets)
  void setIntensityThreshold(uint8_t threshold);              // Set intensity threshold filter (0-255 value), when filtering enable points with intensity below threshold will be filtered
  void setMaxDistance(uint16_t maxDist);                      // Set max distance filter (mm value), when filtering enable points with distance value higher than max will be filtered
  void setMinDistance(uint16_t minDist);                      // Set min distance filter (mm value), when filtering enable points with distance value lower than min will be filtered
  void setDistanceRange(uint16_t minDist, uint16_t maxDist);  // Set both max and min distance filter (mm values)
  void setMaxAngle(int16_t maxAngle);                         // Set max angle filter (integer in °), when filtering enable points with angle value higher than max will be filtered
  void setMinAngle(int16_t minAngle);                         // Set min angle filter (integer in °), when filtering enable points with angle value lower than min will be filtered
  void setAngleRange(int16_t minAngle, int16_t maxAngle);     // Set both max and min angle filter (integers in °)

  // Lidar position parameters
  void setUpsideDown(bool upsideDown = false);  // Set this to true if you put the lidar upside down
#ifdef LD06_COMPUTE_XY
  inline void setBasePosition(int16_t xPos, int16_t yPos, float anglePos) __attribute__((always_inline));  // Set "moving base" position if lidar is on a "moving base", x and y in mm and angle in °
  void setOffsetPosition(int16_t xPos, int16_t yPos, float anglePos);                                      // Set lidar offset positions from position of the "moving base", x and y in mm and angle in °
#endif

  // Getters
  // getSpeed: rotation speed of the last packet, in degrees/s (3600 = 10 turns/s).
  // getAngleStep: angle between 2 measures of the last packet, in degrees.
  // getTimeStamp: LD06 time of the last packet, in ms (wraps around at 30000).
  // getNbPointsInScan / getPoints(n): points of the last COMPLETE scan, n = 0..count-1.
  //   Only call them after readScan() returned true at least once (no scan before).
  //   getPoints() returns an empty point (all 0) for n out of range.
  // getChecksumFailCount: number of packets dropped because of a wrong CRC since startup.
  // getPreviousPacket: raw content of the last valid packet (debug).
  inline uint16_t getSpeed() __attribute__((always_inline));
  inline float getAngleStep() __attribute__((always_inline));
  inline float getTimeStamp() __attribute__((always_inline));
  inline uint16_t getNbPointsInScan() __attribute__((always_inline));
  inline bool isNewScan() __attribute__((always_inline));
  inline DataPoint *getPoints(uint16_t n) __attribute__((always_inline));
  inline uint16_t getChecksumFailCount() __attribute__((always_inline));
  inline const LD06Packet* getPreviousPacket() const __attribute__((always_inline)) {
    return &_previousPacket;
  }

  // Others
  inline bool isChecksumOk() __attribute__((always_inline));  // false if a CRC error happened since the previous call
  int16_t rescaleAngle(int16_t angle);                         // brings an angle back into 0..360 degrees

private:
  bool readData();       // CRC or no CRC version, depending on _useCRC
  bool readDataCRC();
  bool readDataNoCRC();
  void computeData();    // converts the packet just received into DataPoints
  inline bool filter(const DataPoint &point) __attribute__((always_inline));
  void swapBuffers();    // the scan being filled becomes the "previous" (readable) one

  // Data: double buffer, see the description at the top of this file
  DataPointHandler _scanA;
  DataPointHandler _scanB;
  DataPointHandler *_currentScan = &_scanA;  // being filled by computeData()
  DataPointHandler *_previousScan;           // last complete scan, read by the getters. Not set before the first swapBuffers()
  bool _currentBuffer = 0;  // which buffer swapBuffers() selects next

  bool _newScan = false;            // set by computeData() when a scan is complete
  uint16_t _checksumFailCount = 0;

  // Reading buffers
  LD06Packet _previousPacket;        // copy of the last valid packet
  LD06PacketHandler _receivedData;   // packet being received
  float _angles[LD06_PTS_PER_PACKETS];  // angles of the 12 measures of the last packet

  // Settings
  HardwareSerial *_lidarSerial;
  uint8_t _pin;  // PWM pin, 255 = not used
  bool _useCRC = true;
  bool _fullScan = true;
  bool _useFiltering = false;
  bool _upsideDown = false;
  int16_t _xPosition = 0;         // robot ("moving base") position on the table, setBasePosition()
  int16_t _yPosition = 0;
  float _angularPosition = 0.0f;
  int16_t _xOffset = 0;           // lidar position on the robot, setOffsetPosition()
  int16_t _yOffset = 0;
  float _angularOffset = 0.0f;

  // Filtering Settings
  uint16_t _minDist = 0;      // Minimum Distance mm
  uint16_t _maxDist = 12000;  // Maximum Distance mm
  uint16_t _minAngle = 0;     // Minimum angle ° (min > max = range crossing 0°, e.g. 300..60)
  uint16_t _maxAngle = 360;   // Maximum angle °
  uint16_t _threshold = 0;    // Minimum point intensity 0-255
};

// Inline setters
#ifdef LD06_COMPUTE_XY
void LD06::setBasePosition(int16_t xPos = 0, int16_t yPos = 0, float anglePos = 0.0f) {
  _xPosition = xPos;
  _yPosition = yPos;
  _angularPosition = anglePos;
}
#endif

// Inline getters

uint16_t LD06::getNbPointsInScan() {
  return _previousScan->index;
}

uint16_t LD06::getSpeed() {
  return _receivedData.packet.lidarSpeed;
}

float LD06::getAngleStep() {
  float fsa = (float)_receivedData.packet.startAngle / 100.0f;
  float lsa = (float)_receivedData.packet.endAngle / 100.0f;
  float range = lsa - fsa;
  if (range < 0)
    range += 360.0f;

  return range / LD06_PTS_PER_PACKETS;
}

float LD06::getTimeStamp() {
  return _receivedData.packet.timeStamp;
}

bool LD06::isNewScan() {
  return _newScan;
}

DataPoint *LD06::getPoints(uint16_t n) {
  static DataPoint empty;
  DataPoint *result = &empty;
  if (n < _previousScan->index)
    result = &_previousScan->points[n];
  return result;
}

uint16_t LD06::getChecksumFailCount() {
  return _checksumFailCount;
}

// Inline compute
/* Checksum failure check.
   return false only if a failure occured since last time you checked
   WARNING: the line inside the "if" assigns in the wrong direction (it should be
   previousChecksumFailCount = checksumFailCount): after the first CRC error this
   function keeps returning false. Use getChecksumFailCount() until it is fixed.
*/
bool LD06::isChecksumOk() {
  static uint16_t previousChecksumFailCount = 0;
  uint16_t checksumFailCount = getChecksumFailCount();
  if (checksumFailCount != previousChecksumFailCount) {
    checksumFailCount = previousChecksumFailCount;
    return false;
  }
  return true;
}

/* Points filter.
   return : true if point pass the filter
*/
bool LD06::filter(const DataPoint &point) {
  // A point must pass the 3 filters: distance, intensity and angle
  bool distanceFilter = point.distance <= _maxDist && point.distance >= _minDist;
  bool intensityFilter = point.intensity >= _threshold;
  bool angularFilter;
  // min <= max: keep min..max. min > max: the range crosses 0°, keep min..360 and 0..max
  if (_minAngle <= _maxAngle) {
    angularFilter = point.angle <= _maxAngle && point.angle >= _minAngle;
  } else {
    angularFilter = point.angle <= _maxAngle || point.angle >= _minAngle;
  }
  return distanceFilter && intensityFilter && angularFilter;
}

#endif