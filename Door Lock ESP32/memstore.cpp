#include <Arduino.h>
#include "memstore.h"
#include <stdint.h>

/* Proper EEPROM Code in the other file  https://wokwi.com/projects/439077869366072321 */

const int EEPROM_SIZE = 512;
#define EMPTY_MEM           0xff

/* Normal Variables instead of EEPROM Since ESP32 doesnt have memory */
char DOOR_STATE_MEM_POS = ((char)EMPTY_MEM);
char MFA_STATE_MEM_POS[6] = {((char)EMPTY_MEM), ((char)EMPTY_MEM), ((char)EMPTY_MEM), ((char)EMPTY_MEM), ((char)EMPTY_MEM), ((char)EMPTY_MEM)};
int PIN_VALID_MEM_POS = EMPTY_MEM;
char PIN_MEM_POS[4] = {((char)EMPTY_MEM), ((char)EMPTY_MEM), ((char)EMPTY_MEM), ((char)EMPTY_MEM)};
uint8_t RFID_STATUS_MEM_POS = EMPTY_MEM;
String RFID_MEM_POS[3] = {"", "", ""};

/* Map memory location to meaning */
// #define DOOR_STATE_MEM_POS  0  //char
// #define PIR_STATE_MEM_POS   1  //char
// #define PIN_STATE_MEM_POS   2  //char
// #define OTP_STATE_MEM_POS   3  //char
// #define RFID_STATE_MEM_POS  4  //char
// #define PIN_VALID_MEM_POS   5  //int
// #define PIN_MEM_POS         6  //String: char[4]
// #define RFID_STATUS_MEM_POS 10 //uint8_t 
// #define RFID_MEM_POS        11 // 8 * 3 chars


#define CLOSED   ((char)0)
#define OPEN     ((char)1)
#define INACTIVE ((char)2)

MemStore::MemStore() {

}

void MemStore::set_door_state(bool state) {
  char value = (state) ? OPEN : CLOSED;
  DOOR_STATE_MEM_POS = value;
}

bool MemStore::get_door_state() {
  return DOOR_STATE_MEM_POS == OPEN;
}

void MemStore::set_state(MFA state, MFA_STATE value) {
  char v = INACTIVE;
  switch (value) {
    case MFA_STATE::_CLOSED:
      v = CLOSED;
      break;
    case MFA_STATE::_OPEN:
      v = OPEN;
      break;
    default: break;
  }

  MFA_STATE_MEM_POS[static_cast<int>(state)] = v;
}

MFA_STATE MemStore::get_state(MFA state) {
  char value = MFA_STATE_MEM_POS[static_cast<int>(state)];
  switch (value) {
    case CLOSED:
      return MFA_STATE::_CLOSED;
    case OPEN:
      return MFA_STATE::_OPEN;
    case INACTIVE:
      return MFA_STATE::_INACTIVE;
    default:
      // defaults for states if absolute first time
      switch (state) {
        case MFA::PIR:
          return MFA_STATE::_CLOSED;
        case MFA::PIN:
          return MFA_STATE::_CLOSED;
        case MFA::OTP:
          return MFA_STATE::_CLOSED;
        case MFA::RFID:
          return MFA_STATE::_INACTIVE;
        default:
          return MFA_STATE::_OPEN;
      }
  } 
}

bool MemStore::has_pin() {
  return PIN_VALID_MEM_POS != EMPTY_MEM;
}

void MemStore::change_pin(String pin) {
  PIN_VALID_MEM_POS = OPEN;  // OPEN = (char) 1; can be anything else
  for (int i = 0; i < pin.length(); ++i) {
    PIN_MEM_POS[i] = pin[i];
  }
}

bool MemStore::validate_pin(String input) {
  if (!has_pin()) return true; // If no pin, then return true

  for (int i = 0; i < input.length(); ++i) {
    auto digit = PIN_MEM_POS[i];
    if (digit != input[i]) return false;
  }

  return true;
}

String MemStore::read_rfid(int id) {
  uint8_t rfid_status = RFID_STATUS_MEM_POS;
  if (rfid_status & (1 << id) == 0) return "";
  return RFID_MEM_POS[id];
}

RFID_Data MemStore::get_rfids() {
  uint8_t rfid_status = RFID_STATUS_MEM_POS;
  RFID_Data data;
  data.rfid1 = (rfid_status & 0b001) ? read_rfid(0) : "";
  data.rfid2 = (rfid_status & 0b010) ? read_rfid(1) : "";
  data.rfid3 = (rfid_status & 0b100) ? read_rfid(2) : "";
  return data;
}

void MemStore::set_rfid(int id, String data) {
  RFID_MEM_POS[id] = data;

  if (data.length() == 8) {
    RFID_STATUS_MEM_POS |= (1 << id);
  }
}

void MemStore::del_rfid(int id) {
  if (RFID_STATUS_MEM_POS & (1 << id) == 0) return;
  RFID_STATUS_MEM_POS &= (~(1 << id));
}
