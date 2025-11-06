#include <Arduino.h>
#include <EEPROM.h>
#include "memstore.h"
#include <stdint.h>

/* Map memory location to meaning */
#define DOOR_STATE_MEM_POS  0  //char
#define PIR_STATE_MEM_POS   1  //char
#define PIN_STATE_MEM_POS   2  //char
#define OTP_STATE_MEM_POS   3  //char
#define RFID_STATE_MEM_POS  4  //char
#define PIN_VALID_MEM_POS   5  //int
#define PIN_MEM_POS         6  //String: char[4]
#define RFID_STATUS_MEM_POS 10 //uint8_t 
#define RFID_MEM_POS        11 // 8 * 3 chars

#define EMPTY_MEM           0xff

#define CLOSED   ((char)0)
#define OPEN     ((char)1)
#define INACTIVE ((char)2)

MemStore::MemStore() {

}

void MemStore::set_door_state(bool state) {
  char value = (state) ? OPEN : CLOSED;
  EEPROM.write(DOOR_STATE_MEM_POS, value);
}

bool MemStore::get_door_state() {
  char value = EEPROM.read(DOOR_STATE_MEM_POS);
  return value == OPEN;
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

  EEPROM.write(static_cast<int>(state), v);
}

MFA_STATE MemStore::get_state(MFA state) {
  char value = EEPROM.read(static_cast<int>(state));
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
  auto value = EEPROM.read(PIN_VALID_MEM_POS);
  return value != EMPTY_MEM;
}

void MemStore::change_pin(String pin) {
  EEPROM.write(PIN_VALID_MEM_POS, OPEN); // OPEN = (char) 1; can be anything else
  for (byte i = 0; i < pin.length(); ++i) {
    EEPROM.write(PIN_MEM_POS + i, pin[i]);
  }
}

bool MemStore::validate_pin(String input) {
  if (!has_pin()) return true; // If no pin, then return true

  for (byte i = 0; i < input.length(); ++i) {
    auto digit = EEPROM.read(PIN_MEM_POS + i);
    if (digit != input[i]) return false;
  }

  return true;
}

String MemStore::read_rfid(int id) {
  uint8_t rfid_status = EEPROM.read(RFID_STATUS_MEM_POS);
  if (rfid_status & (1 << id) == 0) return "";
  String output = "";
  for (int i = 0; i < 8; ++i) {
    char c =  EEPROM.read(RFID_MEM_POS + (8 * id) + i);
    if (c != (char)0xff) output += c;
  }

  if (output.length() == 8) {
    return output;
  } else {
    return "";
  }
}

RFID_Data MemStore::get_rfids() {
  uint8_t rfid_status = EEPROM.read(RFID_STATUS_MEM_POS);
  RFID_Data data;
  data.rfid1 = (rfid_status & 0b001) ? read_rfid(0) : "";
  data.rfid2 = (rfid_status & 0b010) ? read_rfid(1) : "";
  data.rfid3 = (rfid_status & 0b100) ? read_rfid(2) : "";
  return data;
}

void MemStore::set_rfid(int id, String data) {
  for (int i = 0; i < 8; ++i) {
    EEPROM.write(RFID_MEM_POS + (8 * id) + i, data[i]);
  }

  uint8_t rfid_status = EEPROM.read(RFID_STATUS_MEM_POS);
  rfid_status |= (1 << id);
  EEPROM.write(RFID_STATUS_MEM_POS, rfid_status); 
}

void MemStore::del_rfid(int id) {
  uint8_t rfid_status = EEPROM.read(RFID_STATUS_MEM_POS);
  if (rfid_status & (1 << id) == 0) return;
  rfid_status &= (~(1 << id));
  EEPROM.write(RFID_STATUS_MEM_POS, rfid_status); 
}
