#include "device.h"

MegaSerial mega(MEGA_BAUDRATE);

Device::Device() { Serial.println("Device instance initilized..."); }

WiFiClient* Device::getClient() { return &client; }

void Device::msgHandler(NATS::msg msg) { 
  Serial.println(msg.data);
  std::pair<uint8_t*, uint8_t> res = jsonToBuffer(msg.data);

  if (res.second)
    mega.sendPacket(res.first, res.second);
}

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
    StaticJsonDocument<200> doc = bufferToJson(p.first);
    doc["time"] = getTime();

    String s;
    serializeJson(doc, s);

    uint8_t cmd = p.first[2];

    if(cmd == Commands::SendParkingStatus || Commands::SendTransInfo)
    {
      
      if(strncmp(topic.c_str(),"evm",3) == 0)
      {
        this->topic.remove(0,3);
        this->topic = (String)NATS_TOPIC + this->topic;
      }
    }

    if(cmd == RESP_EVSE_STATE || cmd == RESP_CARD_READ || cmd == RESP_CHARGING_INFO)
    {
      if(strncmp(topic.c_str(),"qparking",8) == 0)
      {
        this->topic.remove(0,8);
        this->topic = (String)"evm" + this->topic;
      }
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
      nats->publish((this->topic += ".PARKING_STATE").c_str(), s);
    }
    else if (cmd == Commands::SendTransInfo)
      nats->request((this->topic += ".TRANS_INFO_REQUEST").c_str(), s, requestCB);
    
    // EVM
    else if (RESP_EVSE_STATE == cmd)
      nats->publish((this->topic += ".RESP_EVSE_STATE").c_str(), s);
    else if (RESP_CARD_READ == cmd)
      nats->request((this->topic += ".RESP_CARD_READ").c_str(), s, requestCB);
    else if (RESP_CHARGING_INFO == cmd)
      nats->publish((this->topic += ".RESP_CHARGING_INFO").c_str(), s);
  }
}