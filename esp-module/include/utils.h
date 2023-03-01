#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "config.h"


extern void printBytes(uint8_t *buffer, uint16_t bufferSize) {
  int a;
    for (int i=0 ; i < bufferSize; ++i) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
    Serial.print("\n");
}

#endif