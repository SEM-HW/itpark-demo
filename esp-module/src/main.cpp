#include "init.h"
#include "utils.h"

void setup() {
  Serial.begin(9600);
  device.connect_wifi();

  nats.on_connect = nats_on_connect;
  nats.connect();

  device.setupTime();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    device.connect_wifi();
    return;
  }
  nats.process();
  yield();

  device.loop();
  yield();
}