#ifndef DATA_TRANSLATOR_H
#define DATA_TRANSLATOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

#define ABCD 0xAB, 0xCD
#define DCBA 0xDC, 0xBA

StaticJsonDocument<200> bufferToJson(uint8_t* buff);
std::pair<uint8_t*, uint8_t> jsonToBuffer(const char* s);
std::pair<uint8_t*, uint8_t> wrapOnlineStatus(uint8_t status);

#endif