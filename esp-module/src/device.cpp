#include "device.h"

MegaSerial mega(MEGA_BAUDRATE);

Device::Device() { Serial.println("Device instance initilized..."); }

WiFiClient* Device::getClient() { return &client; }

void Device::msgHandler(NATS::msg msg) { Serial.println(msg.data); }

void Device::setNats(NATS* nats) { this->nats = nats; }


uint64_t Device::getTime() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL))
    return 0;
  return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
}


uint64_t Device::setupTime() {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET, NTP_SERVER);
  return getTime();
}


void Device::connect_wifi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  int k = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (++k % 1200 == 0)
      Serial.print(".");
    yield();
  }
  Serial.println("");
  Serial.println("WiFi connected.");
}

String Device::initDeviceTopic(bool verbose/* = false*/) {
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  if (verbose) {
    Serial.println("Hardware info");
    Serial.printf("%d cores Wifi %s%s\n", chip_info.cores, (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
      (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    Serial.printf("Silicon revision: %d\n", chip_info.revision);
    Serial.printf("%dMB %s flash\n", spi_flash_get_chip_size() / sq(1024),
      (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embeded" : "external");
  }

  //get chip id
  String chipId = String((uint32_t)ESP.getEfuseMac(), HEX);
  chipId.toUpperCase();

  if (verbose) {
    Serial.printf("Chip id: %s\n", chipId.c_str());
    Serial.println((String)"Mac address is " + WiFi.macAddress());
  }

  return this->topic = ((String)NATS_TOPIC + "." + (String)chipId + "-" + WiFi.macAddress());
}


void requestCB(NATS::msg msg) {
  std::pair<uint8_t*, uint8_t> res = jsonToBuffer(msg.data);

  if (res.second)
    mega.sendPacket(res.first, res.second);
}

void Device::loop() {
  std::pair<uint8_t*, uint8_t> p = mega.readPacket();
  if (p.second) {
    #ifdef DEBUG
    Serial.println("STATUS");
    Serial.println(p.first[4]);
    #endif
    StaticJsonDocument<200> doc = bufferToJson(p.first);
    doc["time"] = getTime();

    String s;
    serializeJson(doc, s);

    uint8_t cmd = p.first[3];

    if(cmd == Commands::SendParkingStatus || Commands::SendTransInfo)
    {
      this->topic.remove(0,3);
      this->topic = (String)NATS_TOPIC + this->topic;
    }

    if(cmd == RESP_EVSE_STATE || cmd == RESP_CARD_READ || cmd == RESP_CHARGING_INFO)
    {
      this->topic.remove(0,8);
      this->topic = (String)"evm" + this->topic;
    }

    if (cmd == Commands::SendParkingStatus) {
      // std::pair<uint8_t*, uint8_t> res = wrapOnlineStatus(
      //   WiFi.status() == WL_CONNECTED ?
      //   OnlineStatus::Online :
      //   WiFi.status() == WL_CONNECTION_LOST || 
      //   WiFi.status() == WL_DISCONNECTED ? 
      //   OnlineStatus::Offline :
      //   WiFi.status() == WL_CONNECT_FAILED ?
      //   OnlineStatus::Error :
      //   OnlineStatus::Connecting
      // );

      // mega.sendPacket(res.first, res.second);
      this->topic += ".PARKING_STATE";
      nats->publish(this->topic.c_str(), s);
      this->topic.remove(35,14);
      return;
    }
    else if (cmd == Commands::SendTransInfo)
    {
      this->topic += ".TRANS_INFO_REQUEST";
      nats->request((const char*)this->topic.c_str(), s, requestCB);
      this->topic.remove(35,19);
      // qparking.F255B594-94:B5:55:F2:6A:7C
    }
    else if (RESP_EVSE_STATE == cmd)
    {
      this->topic += ".RESP_EVSE_STATE";
      nats->publish(this->topic.c_str(), s);
      this->topic.remove(35,16);
    }
    else if (RESP_CARD_READ == cmd)
    {
      this->topic += ".RESP_CARD_READ";
      nats->request((const char*)this->topic.c_str(), s, requestCB);
      this->topic.remove(35,15);
    }
    else if (RESP_CHARGING_INFO == cmd)
    {
      this->topic += ".RESP_CHARGING_INFO";
      nats->publish(this->topic.c_str(), s);
      this->topic.remove(35,19);
    }
  }
}