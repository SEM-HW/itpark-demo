# EVM Device Protocol

## Packet data description

<br/>

- **EVSE State**  - OpenEVSE controller state (1 bytes).
  - Sleeping   - no vehicle
  - Disabled   - vehicle is parked
  - State A  - not connected
  - State B  - connected
  - State C  - charging
  - State D  - error "vent required"
  - Error "diode check failed"
  - Error "no ground"
  - Error "bad ground"
  - Error "stuck relay"
  - Error "GFI failed"
  - Error "over temperature shutdown"
  - Error "over current shutdown"
  - unknown
<br/><br/>

- **Card UID**     - EPass card UID number (4 byte).
- **Unit Price**   - Unit Price per kW (default = 500); Get from EVM Server (tugrik for Mongolia).
- **Energy** - Energy usage in Watts (2 bytes).
- **Amount** - Amount value corresponding to energy usage in kW (4 bytes).
- **Accumulated energy** - Energy usage after Arduino powered on (8 bytes).
- **Charging status**      - corresponding charging status (1 byte):
  - SUCCESS             - Done charging.
  - CHARGING      - User validated card is not registered.
  - Transaction time ERROR  - User did not charging station for 30 seconds.
  - User card Balance ERROR   - User card balance is zero. Charged a vehicle.
  - EVSE ERROR   - Error occured in OpenEVSE controller. May charged a vehicle.
  - Card ERROR   - Unregistered card is read.
  - unknown
- **ACK STATUS** - Acknowledgment status (1 byte):
  - SUCCESS   - Received and Sent charging info (0x36) to server.
  - FAILED    - Recevied but did NOT sent charging info to server.
<br/><br/>

--------------
--------------

Packet example:

```py
# topic:
"F255B594-94:B5:55:F2:6A:7C.<COMMAND_STRING>"

# DEVICE TO SERVER
# command: "RESP_EVSE_STATE",
{
  "state": "Sleeping" | "Disabled" | "State_A" | "State_B" | "State_C" | "State_D" | "Error diode check" | "Error no ground" | "Error bad ground" | "Error stuck relay" | "Error GFI" | "Error over temp" | "Error over current"
}

# command: "RESP_CARD_READ",
{
  "state": "Sleeping" | "Disabled" | "State_A" | "State_B" | "State_C" | "State_D" | "Error diode check" | "Error no ground" | "Error bad ground" | "Error stuck relay" | "Error GFI" | "Error over temp" | "Error over current",
  "cardUid": "3BA3998A"
}

# command: "RESP_CHARGING_INFO",
{
  "state": "Sleeping" | "Disabled" | "State_A" | "State_B" | "State_C" | "State_D" | "Error diode check" | "Error no ground" | "Error bad ground" | "Error stuck relay" | "Error GFI" | "Error over temp" | "Error over current",
  "cardUid": "78B46C2A",
  "price": 500,
  "energy": 10000,
  "amount": 5000,
  "accumulated": 2500,
  "status": "Success" | "Charging" | "Transaction time error" | "User card balance error" | "EVSE error" | "Card error" | "Unknown"
}

# SERVER TO DEVICE
# command: "START_CHARGING",
{
  "cardUid": "78B46C2A",
  "cardBalance": 25000,
  "price": 500
}

# command: "TRANS_ACK",
{
  "state": "Sleeping" | "Disabled" | "State_A" | "State_B" | "State_C" | "State_D" | "Error diode check" | "Error no ground" | "Error bad ground" | "Error stuck relay" | "Error GFI" | "Error over temp" | "Error over current",
  "cardUid": "78B46C2A",
  "price": 500,
  "energy": 10000,
  "amount": 5000,
  "accumulated": 2500,
  "acknowledgement": "SUCCESS" | "FAILED"
}

CommandList: [
  'RESP_EVSE_STATE',
  'RESP_CARD_READ',
  'RESP_CHARGING_INFO',
  'START_CHARGING',
  'TRANS_ACK',
]

StateList: [
  'Sleeping',
  'Disabled',
  'State_A',
  'State_B',
  'State_C',
  "State_D",
  "Error diode check",
  "Error no ground",
  "Error bad ground",
  "Error stuck relay",
  "Error GFI",
  "Error over temp",
  "Error over current",
]

# Charging status list
status: [
  'Success',
  'Charging',
  'Transaction time error',
  'User card balance error',
  'EVSE error',
  "Card error",
  "Unknown",
]

acknowledgement: [
  "SUCCESS",
  "FAILED",
]
```

--------------

