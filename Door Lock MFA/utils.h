#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

enum class MFA {
  PIR  = 1,
  PIN  = 2,
  OTP  = 3,
  RFID = 4,
  NONE = 5
};

enum class MFA_STATE {
  _CLOSED   = 0,
  _OPEN     = 1,
  _INACTIVE = 2
};

struct RFID_Data {
  String rfid1;
  String rfid2;
  String rfid3;
};

#endif