#ifndef MY_DEVICE_H
#define MY_DEVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include "ArduinoNATS.h"
#include "config.h"
#include "time.h"
#include "MegaSerial.h"
#include "DataTranslator.h"

extern MegaSerial mega;

class Device {
  WiFiClient client;
  String topic;
  NATS *nats;
public:
  String initDeviceTopic(bool verbose = false);
  void connect_wifi();
  void msgHandler(NATS::msg msg);
  void loop();
  uint64_t setupTime();
  uint64_t getTime();
  Device();

  void setNats(NATS *nats);
  WiFiClient* getClient();
};

#endif