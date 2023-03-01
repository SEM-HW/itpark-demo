#include "DataTranslator.h"
#include <FastCRC.h>


void buildSendParkingStatus(uint8_t* buff, StaticJsonDocument<200>& doc)
{
  if(buff[4] == UNLOCKED)
    doc["status"] = "UNLOCKED";

  if(buff[4] == LOCKED)
    doc["status"] = "LOCKED";

  if(buff[4] == UNLOCKING)
    doc["status"] = "UNLOCKING";

  if(buff[4] == LOCKING)
    doc["status"] = "LOCKING";

  if(buff[4] == Error)
    doc["status"] = "ERROR";
    
  doc["price"] =  ((buff[7] << 8) | buff[6]);
}


void buildSendTransInfo(uint8_t* buff, StaticJsonDocument<200>& doc)
{
  String cardUid;
  for (int i=5; i<9; ++i) 
  {
    if (buff[i] < 0x10) cardUid += '0';
    cardUid += String(buff[i], HEX);
  }
  cardUid.toUpperCase();

  doc["cardUid"] = cardUid;
}

// EVM
// TODO: CHANGE NUMERICALS VALUES TO STRING
void buildResp_evse(uint8_t* buff, StaticJsonDocument<200>& doc)
{
  doc["state"] = buff[4];
}

void buildResp_card(uint8_t* buff, StaticJsonDocument<200>& doc)
{
  doc["state"] = buff[4];
  
  String cardUid;
  for (int i=5; i<9; ++i) 
  {
    if (buff[i] < 0x10) cardUid += '0';
    cardUid += String(buff[i], HEX);
  }
  cardUid.toUpperCase();

  doc["cardUid"] = cardUid;
}

void buildResp_charging(uint8_t* buff, StaticJsonDocument<200>& doc)
{
  doc["state"] = buff[4];

  String cardUid;
  for (int i=5; i<9; ++i) 
  {
    if (buff[i] < 0x10) cardUid += '0';
    cardUid += String(buff[i], HEX);
  }
  cardUid.toUpperCase();

  doc["cardUid"] = cardUid;

  doc["price"] = ((buff[11] << 8) | buff[10]);

  doc["energy"] = ((buff[13] << 8) | buff[12]);

  doc["amount"] = ((buff[17] << 24) | (buff[16] << 16) | (buff[15] << 8) | buff[14]);

  doc["accumulated"] = ((buff[25] << 56) | (buff[24] << 48) | (buff[23] << 40) | (buff[22] << 32) | (buff[21] << 24) | (buff[20] << 16) | (buff[19] << 8) | buff[18]);

  if(buff[26] == Success)
    doc["status"] = "Success";

  if(buff[26] == Charging)
    doc["status"] = "Charging";

  if(buff[26] == Transaction_time_error)
    doc["status"] = "Transaction time error";  

  if(buff[26] == User_card_balance_error)
    doc["status"] = "User card balance error";

  if(buff[26] == EVSE_error)
    doc["status"] = "EVSE error";

  if(buff[26] == Card_error)
    doc["status"] = "Card error";

  if(buff[26] == Charge_unknown)
    doc["status"] = "Unknown";  
}

StaticJsonDocument<200> bufferToJson(uint8_t* buff)
{
  static StaticJsonDocument<200> doc;

  doc.clear();

  if (buff[2] == SendParkingStatus)         buildSendParkingStatus(buff, doc);
  if (buff[2] == SendTransInfo)             buildSendTransInfo(buff, doc);
  // EVM
  if (buff[2] == RESP_EVSE_STATE)           buildResp_evse(buff, doc);
  if (buff[2] == RESP_CARD_READ)            buildResp_card(buff, doc);
  if (buff[2] == RESP_CHARGING_INFO)        buildResp_charging(buff, doc);
  
  return doc;
}


void insertCRC(uint8_t *buffer, int8_t bufferSize) {
  uint8_t tmp[50];
  Serial.println(bufferSize);
  for(int i=2; i<bufferSize; ++i)
    tmp[i-2] = buffer[i];

  FastCRC16 CRC16;
  uint16_t crc = CRC16.kermit(tmp, bufferSize - 6);

  buffer[bufferSize-3] = highByte(crc);
  buffer[bufferSize-4] = lowByte(crc);
}


std::pair<uint8_t*, uint8_t> wrapOnlineStatus(uint8_t status) {
  const int len = CommandLengths::OnlineStatusLen + 8;
  static uint8_t ans[len] = { ABCD, OnlineStatus, OnlineStatusLen, status, 0x00, 0x00, DCBA };
  insertCRC(ans, len);
  return std::make_pair(ans, len);
}


std::pair<uint8_t*, uint8_t> wrapSetPrice(uint16_t price) {
  const int len = SetPriceLen + 8;
  static uint8_t ans[len] = { ABCD, SetPrice, SetPriceLen, highByte(price), lowByte(price), 0x00, 0x00, DCBA };
  insertCRC(ans, len);
  return std::make_pair(ans, len);
}


std::pair<uint8_t*, uint8_t> wrapChangeParkingStatus(uint8_t status) {
  const int len = ChangeParkingStatusLen + 8;
  static uint8_t ans[len] = { ABCD, ChangeParkingStatus, ChangeParkingStatusLen, status, 0x00, 0x00, DCBA };
  insertCRC(ans, len);
  return std::make_pair(ans, len);
}


std::pair<uint8_t*, uint8_t> wrapTransInfo(uint8_t paymentStatus) {
  const int len = TransInfoLen + 8;

  static uint8_t ans[len] = { ABCD, TransInfo, TransInfoLen, paymentStatus, 0x00, 0x00, DCBA };
  insertCRC(ans, len);
  return std::make_pair(ans, len);
}

// EVM
std::pair<uint8_t*, uint8_t> wrapStartCharging(uint8_t * cardUid, uint32_t cardBalance, uint16_t unitPrice) {
  const int len = START_CHARGING_LEN + 8;

  static uint8_t ans[len] = { ABCD, START_CHARGING, START_CHARGING_LEN, 
                              cardUid[0], cardUid[1], cardUid[2], cardUid[3],
                              (cardBalance & 0xFF000000) >> 24, (cardBalance & 0xFF0000) >> 16, (cardBalance & 0xFF00) >> 8, (cardBalance & 0xFF),
                              ((unitPrice & 0xFF00) >> 8), (unitPrice & 0xFF),
                              0x00, 0x00, DCBA };
  insertCRC(ans, len);
  return std::make_pair(ans, len);
}

std::pair<uint8_t*, uint8_t> wrapTransAck(uint8_t state, uint8_t * cardUid, uint16_t unitPrice, uint16_t energy, uint32_t amount, uint64_t accumulated, uint8_t status) {
  const int len = TRANSACTION_ACK_LEN + 8;

  static uint8_t ans[len] = { ABCD, TRANSACTION_ACK, TRANSACTION_ACK_LEN, 
                              state, 
                              cardUid[0], cardUid[1], cardUid[2], cardUid[3], 
                              ((unitPrice & 0xFF00) >> 8), (unitPrice & 0xFF), 
                              ((energy & 0xFF00) >> 8), (energy & 0xFF), 
                              (amount & 0xFF000000) >> 24, (amount & 0xFF0000) >> 16, (amount & 0xFF00) >> 8, (amount & 0xFF),
                              (accumulated & 0xFF00000000000000) >> 56, (accumulated & 0xFF000000000000) >> 48, (accumulated & 0xFF0000000000) >> 40, (accumulated & 0xFF00000000) >> 32,
                              (accumulated & 0xFF000000) >> 24,         (accumulated & 0xFF0000) >> 16,         (accumulated & 0xFF00) >> 8,          (accumulated & 0xFF),
                              status,
                              0x00, 0x00, DCBA };
  insertCRC(ans, len);
  return std::make_pair(ans, len);
}


void cardUidFromStr(String cardUid, uint8_t* card) {
  if (cardUid.length() != 8) return;
  for (int i=0; i<8; i+=2) 
    sscanf(((String)"0x" + cardUid.substring(i, i+2)).c_str(), "%x", &card[i>>1]);
}

void cardUidtoHex(String carduid, uint8_t * cardUid)
{
  carduid.toLowerCase();
  char c[9];
  carduid.toCharArray(c,9);

  for(int i = 0; i < 8; i++)
  {
    if(c[i] <= 'f' && c[i] >= 'a')
    {
      c[i] -= 87;
    }
    else if(c[i] <= '9' && c[i] >= '0')
    {
      c[i] -= 48;
    }
  }
  
  cardUid[0] = (c[0] << 4) | c[1];
  cardUid[1] = (c[2] << 4) | c[3];
  cardUid[2] = (c[4] << 4) | c[5];
  cardUid[3] = (c[6] << 4) | c[7];
}

std::pair<uint8_t*, uint8_t> jsonToBuffer(const char* s) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, s);

  if (error) {
    Serial.println((String)"Can\'t deserialize json with code: " + error.c_str());
    return std::make_pair(nullptr, 0);
  }
  
  if (doc.containsKey("price"))         
    return wrapSetPrice(doc["price"]);

  if (doc.containsKey("status"))
  {
    uint8_t status = Error;

    if(strncmp(doc["status"],"UNLOCKED",9) == 0)
      status = UNLOCKED;

    if(strncmp(doc["status"],"LOCKED",7) == 0)
      status = LOCKED;

    if(strncmp(doc["status"],"UNLOCKING",10) == 0)
      status = UNLOCKING;

    if(strncmp(doc["status"],"LOCKING",8) == 0)
      status = LOCKING;
      
    if(strncmp(doc["status"],"ERROR",6) == 0)
      status = Error;

    return wrapChangeParkingStatus(status);
  }        
  

  if (doc.containsKey("paymentStatus"))
  {
    uint8_t status = 0xFF;
    
    if(strncmp(doc["status"],"SUCCESS",8) == 0)
      status = 0xAA;
    if(strncmp(doc["status"],"NOT_REGISTERED",15) == 0)
      status = 0xE0;
    if(strncmp(doc["status"],"CARD_BALANCE_ERROR",19) == 0)
      status = 0xE1;
    if(strncmp(doc["status"],"SERVER_SIDE_ERROR",18) == 0)
      status = 0xE2;
    if(strncmp(doc["status"],"FAILED",7) == 0)
      status = 0xFF;

    return wrapTransInfo(status);
  }

  // EVM
  if (doc.containsKey("cardBalance"))
  {
    String carduid = doc["cardUid"];
    uint8_t cardUid[4] = {0};
    cardUidtoHex(carduid, cardUid);
    uint32_t cardBalance = doc["cardBalance"];
    uint16_t unitPrice = doc["price"];
    return wrapStartCharging(cardUid, cardBalance, unitPrice);
  }

  if(doc.containsKey("energy"))
  {
    uint8_t status = Unknown;

    if(strncmp(doc["state"],"Sleeping",8) == 0)
      status = Sleeping;

    if(strncmp(doc["state"],"Disabled",8) == 0)
      status = Disabled;

    if(strncmp(doc["state"],"State_A",7) == 0)
      status = State_A;

    if(strncmp(doc["state"],"State_B",7) == 0)
      status = State_B;
      
    if(strncmp(doc["state"],"State_C",7) == 0)
      status = State_C;
    
    if(strncmp(doc["state"],"State_D",7) == 0)
      status = State_D;

    if(strncmp(doc["state"],"Error diode check",17) == 0)
      status = Diode_check_failed;

    if(strncmp(doc["state"],"Error no ground",15) == 0)
      status = No_ground;

    if(strncmp(doc["state"],"Error bad ground",16) == 0)
      status = Bad_ground;
      
    if(strncmp(doc["state"],"Error stuck relay",17) == 0)
      status = Stuck_relay;

    if(strncmp(doc["state"],"Error GFI",9) == 0)
      status = GFI_failed;

    if(strncmp(doc["state"],"Error over temp",15) == 0)
      status = Over_temperature_shutdown;
      
    if(strncmp(doc["state"],"Error over current",18) == 0)
      status = Over_current_shutdown;
    
    String carduid = doc["cardUid"];
    uint8_t cardUid[4] = {0};
    cardUidtoHex(carduid, cardUid);

    uint16_t unitPrice = (uint16_t)doc["price"];

    uint16_t energy = (uint16_t)doc["energy"];

    uint32_t amount = (uint32_t)doc["amount"];

    uint64_t accumulated = (uint64_t)doc["accumulated"];

    uint8_t ack_status = Ack_failed;

    if(strncmp(doc["acknowledgement"], "SUCCESS", 7) == 0)
      ack_status = Ack_success;

    if(strncmp(doc["acknowledgement"], "FAILED", 7) == 0)
      ack_status = Ack_failed;

    return wrapTransAck(status, cardUid, unitPrice, energy, amount, accumulated, ack_status);
  }

  return std::make_pair(nullptr, 0);
}