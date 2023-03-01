#ifndef INIT_H
#define INIT_H

#include <Arduino.h>
#include <WiFi.h>
#include "ArduinoNATS.h"
#include "device.h"

void nats_echo_handler(NATS::msg msg);
void nats_on_connect();

extern Device device;
extern NATS nats;

#endif