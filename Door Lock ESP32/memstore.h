#ifndef MEMSTORE_H
#define MEMSTORE_H

#include "utils.h"
#include <Arduino.h>

class MemStore {
  public:
    MemStore();
    void set_door_state(bool state);
    bool get_door_state();
    void set_state(MFA state, MFA_STATE value);
    MFA_STATE get_state(MFA state);
    bool has_pin();
    void change_pin(String pin);
    bool validate_pin(String input);
    RFID_Data get_rfids();
    void set_rfid(int id, String data);
    void del_rfid(int id);

  private:
    String read_rfid(int id);
};

#endif