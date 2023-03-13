#include "MegaSerial.h"

MegaSerial::MegaSerial(long baudrate) {
#ifndef SERIAL2
  Serial.begin(baudrate);
  Serial2.begin(baudrate);
#else
  Serial2.begin(baudrate, SERIAL_8N1, RX2, TX2);
#endif
}

std::pair<uint8_t*, uint8_t> MegaSerial::requestPacket(uint8_t buffer[], uint8_t bufferSize, uint16_t timeout) {
  uint16_t startReq = millis();
  sendPacket(buffer, bufferSize);
  if (millis() - startReq > timeout)
    return std::make_pair(nullptr, 0);
  
  return readPacket(timeout - (millis() - startReq));
}

void MegaSerial::sendPacket(uint8_t buffer[], uint8_t bufferSize) {
  Serial2.write(buffer, bufferSize);
}

std::pair<uint8_t*, uint8_t> MegaSerial::readPacket(uint16_t timeout) {
  // uint8_t incomingBytes[MAX_INCOMMING_LEN];

  for (int i = 0; i < MAX_INCOMMING_LEN; ++i)
    incomingBytes[i] = 0;

  Serial2.setTimeout(timeout);

  if (Serial2.available()) {
    uint8_t len = Serial2.readBytes(incomingBytes, MAX_INCOMMING_LEN);
    // for(int i = 0; i<56;i++)
    // {
    //   Serial.println(incomingBytes[i],HEX);
    // }
    if (incomingBytes[0] == 0xAB && incomingBytes[1] == 0xCD)
      if (checkCRC(incomingBytes, len))
        return std::make_pair(incomingBytes, len);
  }

  return std::make_pair(nullptr, 0);
}

bool MegaSerial::checkCRC(uint8_t* buffer, uint8_t size) {
  uint8_t temp[MAX_INCOMMING_LEN];

  for (uint8_t i = 2; i < size - 3; ++i)
    temp[i - 2] = buffer[i];

  uint16_t CRCchecked = this->CRC16.kermit(temp, size - 6);
  uint16_t CRCreceived = makeWord(buffer[size - 3], buffer[size - 4]);

  if (CRCchecked == CRCreceived)
    return true;
  else
    return false;
}