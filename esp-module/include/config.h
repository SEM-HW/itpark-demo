#ifndef CONFIG_H
#define CONFIG_H

// #define WIFI_SSID "SEM-SOFT"
// #define WIFI_PSK "janjinshugamyum"

#define WIFI_SSID "SEM-HW"
#define WIFI_PSK "janjinshugam123"

#define NATS_URL_SERVER "nats.clix.mn"
#define NATS_NAME "qparking:device"
#define NATS_PSWD "Cd4l1vJC4^2G3Cjh4$mVm"
#define NATS_TOPIC "qparking"

#define NTP_SERVER "pool.ntp.org"           // just a ntp server
#define GMT_OFFSET 8                        // Ulaanbaatar timezone
#define GMT_OFFSET_SEC GMT_OFFSET * 60 * 60 // Ulaanbaatar timezone
#define DAYLIGHT_OFFSET 0                   // daylight offset is not used in Mongolia

#define MEGA_BAUDRATE 9600

extern uint8_t device_EUI[16];               //64 bit globally unique Extended Unique Identifier (EUI-64) assigned by the manufacturer, or the owner, of the end-device 

enum Commands
{
    SendParkingStatus = 0x69,
    OnlineStatus = 0x70,
    SetPrice = 0x71,
    ChangeParkingStatus = 0x72,
    SendTransInfo = 0x73,
    TransInfo = 0x74,
};

enum CommandLengths
{
    SendParkingStatusLen = 0x04,
    OnlineStatusLen = 0x01,
    SetPriceLen = 0x02,
    ChangeParkingStatusLen = 0x01,
    SendTransInfoLen = 0x0A,
    TransInfoLen = 0x01,
};

enum OnlineStatus
{
    Online = 0x00,
    Connecting = 0x01,
    Offline = 0x0F,
    Error = 0xEE,
};

enum ParkingStatus
{
    UNLOCKED = 0x00,
    LOCKED = 0x01,
    UNLOCKING = 0x10,
    LOCKING = 0x11,
};

// EVSE STATE
#define Sleeping                    0xFE
#define Disabled                    0xFF
#define State_A                     0x01
#define State_B                     0x02
#define State_C                     0x03
#define State_D                     0x04
#define Diode_check_failed          0x05
#define No_ground                   0x06
#define Bad_ground                  0x07
#define Stuck_relay                 0x08
#define GFI_failed                  0x09
#define Over_temperature_shutdown   0x0A
#define Over_current_shutdown       0x0B
#define Unknown                     0x00

// Charging status

#define Success                     0x01
#define Charging                    0x02
#define Transaction_time_error      0xE1
#define User_card_balance_error     0xE2
#define EVSE_error                  0xE3
#define Card_error                  0xE4
#define Charge_unknown              0x0F

// Ack status

#define Ack_success                 0x11
#define Ack_failed                  0xFF

// Command name

#define RESP_EVSE_STATE             0x32
#define RESP_CARD_READ              0x34
#define RESP_CHARGING_INFO          0x33
#define START_CHARGING              0x36
#define TRANSACTION_ACK             0x37

#define START_CHARGING_LEN          0x0A
#define TRANSACTION_ACK_LEN         0x16


#endif
