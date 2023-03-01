#ifndef MEGA_SERIAL_H
#define MEGA_SERIAL_H

#include <Arduino.h>
#include <FastCRC.h>

#ifdef SERIAL1
#error Hardware serial-1 is not available for esp32!
#endif

#ifdef SERIAL2
#define TX2 17
#define RX2 16
#endif

#define MAX_INCOMMING_LEN 56

class MegaSerial {
  FastCRC16 CRC16;

  bool checkCRC(uint8_t* buffer, uint8_t bufferSize);

public:
  MegaSerial(long baudrate);
  void sendPacket(uint8_t buffer[], uint8_t bufferSize);
  std::pair<uint8_t*, uint8_t> readPacket(uint16_t timeout=5);
  std::pair<uint8_t*, uint8_t> requestPacket(uint8_t buffer[], uint8_t bufferSize, uint16_t timeout=500);
};



#endif