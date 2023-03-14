#include "init.h"

Device device;

NATS nats(device.getClient(),
	    NATS_URL_SERVER, NATS_DEFAULT_PORT, false, NATS_NAME, NATS_PSWD);

void nats_echo_handler(NATS::msg msg) {
  Serial.print(msg.data);
  device.msgHandler(msg);
}

void nats_on_connect() {
  String s_topic = device.initDeviceTopic(false);
  
  device.setNats(&nats);
  Serial.printf("Device topic: %s\n", s_topic.c_str());
  nats.subscribe(s_topic.c_str(), nats_echo_handler);
  
  s_topic += ".SET_PARKING_STATUS";
  nats.subscribe(s_topic.c_str(), nats_echo_handler);
  s_topic.remove(35, 19);

  s_topic += ".SET_PRICE";
  nats.subscribe(s_topic.c_str(), nats_echo_handler);
  s_topic.remove(35, 10);

  s_topic += ".TRANS_INFO_RESPONSE";
  nats.subscribe(s_topic.c_str(), nats_echo_handler);
  s_topic.remove(35, 20);
}