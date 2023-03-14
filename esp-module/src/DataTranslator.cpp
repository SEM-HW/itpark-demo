#include "DataTranslator.h"
#include <FastCRC.h>


void buildSendParkingStatus(uint8_t* buff, StaticJsonDocument<200>& doc)
{
  if(buff[4] == UNLOCKED)   doc["status"] = "UNLOCKED";
  if(buff[4] == LOCKED)     doc["status"] = "LOCKED";
  if(buff[4] == UNLOCKING)  doc["status"] = "UNLOCKING";
  if(buff[4] == LOCKING)    doc["status"] = "LOCKING";
  if(buff[4] == Error)      doc["status"] = "ERROR";
    
  doc["price"] =  ((buff[6] << 8) | buff[7]);
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

String evmState(uint8_t state)
{
  if(state == Sleeping)                   return "Sleeping";
  if(state == Disabled)                   return "Disabled";
  if(state == State_A)                    return "State_A";
  if(state == State_B)                    return "State_B";
  if(state == State_C)                    return "State_C";
  if(state == State_D)                    return "State_D";
  if(state == Diode_check_failed)         return "Diode check failed";
  if(state == No_ground)                  return "No ground";
  if(state == Bad_ground)                 return "Bad ground";
  if(state == Stuck_relay)                return "Stuck relay";
  if(state == GFI_failed)                 return "GFI failed";
  if(state == Over_temperature_shutdown)  return "Over temperature shutdown";
  if(state == Over_current_shutdown)      return "Over current shutdown";
  if(state == Unknown)                    return "Unknown";
  return "Unknown";
}

String evmStatus(uint8_t status)
{
  if(status == Success)                   return "Success";
  if(status == Charging)                  return "Charging";
  if(status == Transaction_time_error)    return "Transaction time error";  
  if(status == User_card_balance_error)   return "User card balance error";
  if(status == EVSE_error)                return "EVSE error";
  if(status == Card_error)                return "Card error";
  if(status == Charge_unknown)            return "Unknown";
  return "Unknown";
}


void buildResp_evse(uint8_t* buff, StaticJsonDocument<200>& doc)
{
  doc["state"] = evmState(buff[4]);
}

void buildResp_card(uint8_t* buff, StaticJsonDocument<200>& doc)
{
  doc["state"] = evmState(buff[4]);
  
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
  doc["state"] = evmState(buff[4]);

  String cardUid;
  for (int i=5; i<9; ++i) 
  {
    if (buff[i] < 0x10) cardUid += '0';
    cardUid += String(buff[i], HEX);
  }
  cardUid.toUpperCase();

  doc["cardUid"] = cardUid;

  doc["price"] = ((buff[10] << 8) | buff[11]);

  doc["energy"] = ((buff[12] << 8) | buff[13]);

  doc["amount"] = ((buff[14] << 24) | (buff[15] << 16) | (buff[16] << 8) | buff[17]);

  doc["accumulated"] = ((buff[18] << 56) | (buff[19] << 48) | (buff[20] << 40) | (buff[21] << 32) | (buff[22] << 24) | (buff[23] << 16) | (buff[24] << 8) | buff[25]);

  doc["status"] = evmStatus(buff[26]); 
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
  for(int i = 0; i < len; i++)
  {
    Serial.println(ans[i],HEX);
  }
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
                              (cardBalance & (0xFFU << 24)) >> 24, (cardBalance & (0xFFU << 16)) >> 16, (cardBalance & (0xFFU << 8)) >> 8, (cardBalance & 0xFFU),
                              ((unitPrice & (0xFFU << 8)) >> 8), (unitPrice & 0xFFU),
                              0x00, 0x00, DCBA };
  insertCRC(ans, len);
  return std::make_pair(ans, len);
}

std::pair<uint8_t*, uint8_t> wrapTransAck(uint8_t state, uint8_t * cardUid, uint16_t unitPrice, uint16_t energy, uint32_t amount, uint64_t accumulated, uint8_t status) {
  const int len = TRANSACTION_ACK_LEN + 8;

  static uint8_t ans[len] = { ABCD, TRANSACTION_ACK, TRANSACTION_ACK_LEN, 
                              state, 
                              cardUid[0], cardUid[1], cardUid[2], cardUid[3], 
                              ((unitPrice & (0xFFU << 8)) >> 8), (unitPrice & 0xFFU), 
                              ((energy & (0xFFU << 8)) >> 8), (energy & 0xFFU), 
                              (amount & (0xFFU << 24)) >> 24, (amount & (0xFFU << 16)) >> 16, (amount & (0xFFU << 8)) >> 8, (amount & 0xFFU),
                              (accumulated & (0xFFU << 56)) >> 56, (accumulated & (0xFFU << 48)) >> 48, (accumulated & (0xFFU << 40)) >> 40, (accumulated & (0xFFU << 32)) >> 32,
                              (accumulated & (0xFFU << 24)) >> 24, (accumulated & (0xFFU << 16)) >> 16, (accumulated & (0xFFU << 8)) >> 8,   (accumulated & 0xFFU),
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

    if(doc["status"] == "UNLOCKED")
      status = UNLOCKED;

    if(doc["status"] == "LOCKED")
      status = LOCKED;

    if(doc["status"] == "UNLOCKING")
      status = UNLOCKING;

    if(doc["status"] == "LOCKING")
      status = LOCKING;
      
    if(doc["status"] == "ERROR")
      status = Error;

    return wrapChangeParkingStatus(status);
  }        
  

  if (doc.containsKey("paymentStatus"))
  {
    uint8_t status = FAILED;
    
    if(doc["status"] == "SUCCESS")
      status = SUCCESS;
    if(doc["status"] == "NOT_REGISTERED")
      status = NOT_REGISTERED;
    if(doc["status"] == "CARD_BALANCE_ERROR")
      status = CARD_BALANCE_ERROR;
    if(doc["status"] == "SERVER_SIDE_ERROR")
      status = SERVER_SIDE_ERROR;
    if(doc["status"] == "FAILED")
      status = FAILED;

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

    if(doc["state"] == "Sleeping")
      status = Sleeping;

    if(doc["state"] == "Disabled")
      status = Disabled;

    if(doc["state"] == "State_A")
      status = State_A;

    if(doc["state"] == "State_B")
      status = State_B;
      
    if(doc["state"] == "State_C")
      status = State_C;
    
    if(doc["state"] == "State_D")
      status = State_D;

    if(doc["state"] == "Error diode check")
      status = Diode_check_failed;

    if(doc["state"] == "Error no ground")
      status = No_ground;

    if(doc["state"] == "Error bad ground")
      status = Bad_ground;
      
    if(doc["state"] == "Error stuck relay")
      status = Stuck_relay;

    if(doc["state"] == "Error GFI")
      status = GFI_failed;

    if(doc["state"] == "Error over temp")
      status = Over_temperature_shutdown;
      
    if(doc["state"] == "Error over current")
      status = Over_current_shutdown;
    
    String carduid = doc["cardUid"];
    uint8_t cardUid[4] = {0};
    cardUidtoHex(carduid, cardUid);

    uint16_t unitPrice = (uint16_t)doc["price"];

    uint16_t energy = (uint16_t)doc["energy"];

    uint32_t amount = (uint32_t)doc["amount"];

    uint64_t accumulated = (uint64_t)doc["accumulated"];

    uint8_t ack_status = Ack_failed;

    if(doc["acknowledgement"] == "SUCCESS")
      ack_status = Ack_success;

    if(doc["acknowledgement"] == "FAILED")
      ack_status = Ack_failed;

    return wrapTransAck(status, cardUid, unitPrice, energy, amount, accumulated, ack_status);
  }

  return std::make_pair(nullptr, 0);
}