#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <SPI.h>
//#include <MFRC522.h>
#include "icons.h"
#include "memstore.h"


// Servo
#define SERVO_PIN        3
#define SERVO_LOCK_POS   20
#define SERVO_UNLOCK_POS 90
Servo lockServo;

// Display
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Keypad
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {4, 5, 6, 7};
byte colPins[KEYPAD_COLS] = {A0, A1, A2, A3};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

// EEPROM
MemStore memstore;

// PIR 
#define PIR_PIN 2

// RFID
//MFRC522 rfid(9, 10);

// Current State
MFA currentState;

// OTP
String otp;

// Settings
int settings = 0;

void lock() {
  lockServo.write(SERVO_LOCK_POS);
  memstore.set_door_state(false);
}

void unlock() {
  lockServo.write(SERVO_UNLOCK_POS);
  memstore.set_door_state(true);
}

void lcd_put_char_at(int col, int row, char c) {
  lcd.setCursor(col, row);
  lcd.write(c);
}

void showStartupMessage() {
  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print("Welcome!");
  delay(1000);

  lcd.setCursor(3, 2);
  String message = "MFA Door Lock!";
  for (byte i = 0; i < message.length(); i++) {
    lcd.print(message[i]);
    delay(100);
  }
  delay(500);
}

MFA init_state() {
  for (int i = 1; i < 5; ++i) {
    MFA state = static_cast<MFA>(i);
    if (memstore.get_state(state) == MFA_STATE::_CLOSED) {
      return state;
    }
  }
  return MFA::NONE;
}

String state_to_string(int i) {
  String s = "";
  switch (i) {
    case 1:
      s = "PIR ";
      break;
    case 2:
      s = "PIN ";
      break;
    case 3:
      s = "OTP ";
      break;
    case 4:
      s = "RFID";
      break;
  } 
  return s;
}

void display_first_line() {
  lcd.clear();
  lcd.setCursor(0, 0);

  for (int i = 1; i < 5; ++i) {
    switch (memstore.get_state(static_cast<MFA>(i))) {
      case MFA_STATE::_CLOSED:
        lcd.write(ICON_LOCKED_CHAR);
        break;
      case MFA_STATE::_OPEN:
        lcd.write(ICON_UNLOCKED_CHAR);
        break;
      case MFA_STATE::_INACTIVE:
        lcd.write(ICON_CROSS_CHAR);
        break;
    }
    String s = state_to_string(i);
    lcd.print(s);
  }
}

void display_locked_state() {
  display_first_line();
  
  lcd.setCursor(4, 1);
  lcd.print("CURRENT: ");
  String s = state_to_string(static_cast<int>(currentState));
  lcd.print(s);

  lcd.setCursor(5, 2);
  lcd.print("# TO RESET");
}

void display_unlocked_state() {
  display_first_line();

  lcd.setCursor(2, 1);
  lcd.write(ICON_UNLOCKED_CHAR);
  lcd.print("DOOR UNLOCKED!");
  lcd.write(ICON_UNLOCKED_CHAR);
  lcd.setCursor(5, 2);
  lcd.print("# TO LOCK");
  lcd.setCursor(3, 3);
  lcd.print("A FOR OPTIONS");
}

void display_settings() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1: ENABLE ");
  lcd.print(state_to_string(settings));
  lcd.setCursor(0, 1);
  lcd.print("2: DISABLE ");
  lcd.print(state_to_string(settings));
  if (settings == 2 || settings == 4) {
    lcd.setCursor(0, 2);
    lcd.print("3: MODIFY ");
    lcd.print(state_to_string(settings));
  }
  lcd.setCursor(0, 3);
  lcd.print("0: CANCEL ALL ");
}

void transition_animation(int delayMilli) {
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("[..........]");
  lcd.setCursor(4, 2);
  lcd.print("[..........]");
  for (int i = 5; i < 15; ++i) {
    lcd_put_char_at(i, 1, '=');
    lcd_put_char_at(i, 2, '=');
    delay(delayMilli);
  }
}

void display_rfid() {
  lcd.clear();
  RFID_Data data = memstore.get_rfids();
  lcd.setCursor(0, 0); 
  lcd.print("1: ");
  lcd.print(data.rfid1);

  lcd.setCursor(0, 1); 
  lcd.print("2: ");
  lcd.print(data.rfid2);

  lcd.setCursor(0, 2); 
  lcd.print("3: ");
  lcd.print(data.rfid3);

  lcd.setCursor(0, 3); 
  lcd.print("0: GO BACK");
}

void display_rfid_settings() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1: VIEW RFID");
  lcd.setCursor(0, 1);
  lcd.print("2: ADD RFID");
  lcd.setCursor(0, 2);
  lcd.print("3: DEL RFID");
  lcd.setCursor(0, 3);
  lcd.print("0: GO BACK");
}

void reset_lock() {
  if (currentState == MFA::NONE) lock();

  for (int i = 1; i < 5; ++i) {
    MFA state = static_cast<MFA>(i);
    if (memstore.get_state(state) == MFA_STATE::_OPEN) {
      memstore.set_state(state, MFA_STATE::_CLOSED);
    }
  }

  transition_animation(200);
  currentState = init_state();
  if (currentState == MFA::NONE) {
    unlock();
    display_unlocked_state();
  } else {
    lock();
    display_locked_state();
  }
}

String generate_otp() {
  String s = "";
  for (int i = 0; i < 4; ++i) {
    s += char('0' + random(0, 10));
  }
  return s;
}

void move_to_next_state() {
  memstore.set_state(currentState, MFA_STATE::_OPEN);
  int position = (static_cast<int>(currentState) * 4) - 4;
  lcd.setCursor(position, 0);
  lcd.write(ICON_UNLOCKED_CHAR);
  currentState = init_state();
  if (currentState != MFA::NONE) {
    display_locked_state();
  } else {
    unlock();
    display_unlocked_state();
    settings = 0;
  }

  if (currentState == MFA::OTP) {
    lcd.setCursor(0, 3);
    lcd.print("OTP ON SERIAL ");
    otp = generate_otp();

    Serial.print("OTP: ");
    Serial.println(otp);
  }

  if (currentState == MFA::RFID) {
    lcd.setCursor(0,3);
    lcd.print("ENTER RFID ON SERIAL");
  }
}

String input_pin(int col, int row) {
  lcd.setCursor(col, row);
  lcd.print("[____]");
  lcd.setCursor(col + 1, row);
  String result = "";
  while (result.length() < 4) {
    char key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      lcd.print('*');
      result += key;
    } else if (key == '#') {
      return result;
    }
  }
  return result;
}

bool set_new_pin() {
  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("Enter new PIN:");
  String newCode = input_pin(7, 2);
  if (newCode.length() != 4) return false;

  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("Confirm new PIN");
  String confirmCode = input_pin(7, 2);
  if (confirmCode.length() != 4) return false;

  if (newCode.equals(confirmCode)) {
    memstore.change_pin(newCode);
    return true;
  } else {
    lcd.clear();
    lcd.setCursor(3, 1);
    lcd.print("PIN mismatch");
    delay(2000);
    return false;
  }
}

bool set_new_rfid() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("ENTER RFID IN SERIAL");
  while (true) {
    if (Serial.available() > 0) {
      String uid = Serial.readStringUntil('\n');
      uid.trim();
      if (uid.length() != 8) {
        lcd.clear();
        lcd.setCursor(3, 1);
        lcd.print("RFID Mismatch");
        Serial.print("MISMATCH ");
        Serial.println(uid);
        delay(2000);
        return false;
      } else {
        display_rfid();
        lcd.setCursor(0, 3);
        lcd.print("REPLACE / 0: GO BACK");
        auto key = keypad.getKey();
        while (key > '3' || key < '0') {
          key = keypad.getKey();
        }
        switch(key) {
          case '1':
            memstore.set_rfid(0, uid);
            break;
          case '2':
            memstore.set_rfid(1, uid);
            break;
          case '3':
            memstore.set_rfid(2, uid);
            break;
          case '0':
            break;
        }
        return true;
      }
    }

    auto key1 = keypad.getKey();
    if (key1 == 'A') {
      break;
    }
  }
  return true;
}

void delete_rfid() {
  auto key = keypad.getKey();
  while (key > '3' || key < '0') {
    key = keypad.getKey();
  }
  if (key != '0')
    memstore.del_rfid(int(key - '0') - 1);
}

void handle_rfid_settings() {
  while (true) {
    display_rfid_settings();
    
    auto key = keypad.getKey();
    while (key > '3' || key < '0') {
      key = keypad.getKey();
    }
    
    switch(key) {
      case '1': {
        display_rfid();
        auto key2 = keypad.getKey();
        while (key2 != '0') {
          key2 = keypad.getKey();
        }
        break;
      }
      case '2': {
        while (!set_new_rfid());
        display_rfid_settings();
        break;
      }
      case '3': {
        display_rfid();
        delete_rfid();
        break;
      }
      case '0': {
        return;
      }
    }
  }
}

void handle_settings_modify() {
  if (settings != 2 && settings != 4) {
    return;
  }

  if (settings == 2) {
    set_new_pin();
    display_settings();
  } else {
    handle_rfid_settings();
    display_settings();
  }

}

void handle_pir() {
  while (true){
    auto key = keypad.getKey();
    int pir_val = digitalRead(PIR_PIN);

    if (key == '#') {
      reset_lock();
      break;
    }
    if (pir_val == HIGH) {
      transition_animation(100);
      lcd.clear();
      lcd.setCursor(4, 1);
      lcd.print("PIR SUCCESS!");
      delay(200);
      move_to_next_state();
      break;
    }
  }
}

void handle_pin() {
  String user_input = input_pin(7, 3);

  if (user_input.length() != 4){
    reset_lock();
    return;
  } 

  if (memstore.validate_pin(user_input)) {
    transition_animation(100);
    lcd.clear();
    lcd.setCursor(4, 1);
    lcd.print("PIN SUCCESS!");
    delay(200);
    move_to_next_state();
    return;
  } else {
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print("Access Denied!");
    delay(1000);
    display_locked_state();
  }
}

void handle_otp() {
  String user_input = input_pin(14, 3);

  if (user_input.length() != 4) {
    reset_lock();
    return;
  } 

  if (user_input.equals(otp)) {
    transition_animation(100);
    lcd.clear();
    lcd.setCursor(4, 1);
    lcd.print("OTP SUCCESS!");
    delay(200);
    move_to_next_state();
    return;
  } else {
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print("Access Denied!");
    delay(1000);
    display_locked_state();
  }
}

void handle_rfid() {
  RFID_Data rfids = memstore.get_rfids();
  if (rfids.rfid1 == "" && rfids.rfid2 == "" && rfids.rfid3 == "") {
    Serial.println("No RFIDS in Memory - Moving to next state");
    transition_animation(100);
    lcd.clear();
    lcd.setCursor(4, 1);
    lcd.print("RFID SUCCESS!");
    delay(200);
    move_to_next_state();
    return;
  }
  while (true) {
    auto key = keypad.getKey();
    if (key == '#') {
      reset_lock();
      break;
    }

    if (Serial.available() > 0) {
      String uid = Serial.readStringUntil('\n');
      uid.trim();

      if ((rfids.rfid1 != "" && rfids.rfid1.equals(uid)) ||
          (rfids.rfid2 != "" && rfids.rfid2.equals(uid)) ||
          (rfids.rfid3 != "" && rfids.rfid3.equals(uid)) ) {
        
        transition_animation(100);
        lcd.clear();
        lcd.setCursor(4, 1);
        lcd.print("RFID SUCCESS!");
        delay(200);
        move_to_next_state();
      } else {
        lcd.clear();
        lcd.setCursor(2, 1);
        lcd.print("Access Denied!");
        delay(1000);
        display_locked_state();
        lcd.setCursor(0,3);
        lcd.print("ENTER RFID ON SERIAL");
      }
    }
  }
}

void handle_unlocked() {
  while (true) {
    if (settings == 0) {
      auto key = keypad.getKey();
      while (key != 'A' && key != '#') {
        key = keypad.getKey();
      }

      if (key == '#') {
        reset_lock();
        return;
      } else {
        settings = 1;
        display_settings();
      }
    } else {
      if (settings == 5) {
        display_unlocked_state();
        settings = 0;
      }
      auto key = keypad.getKey();
      switch (key) {
        case '0':
          display_unlocked_state();
          settings = 0;
          break;
        case '1':
          memstore.set_state(static_cast<MFA>(settings), MFA_STATE::_OPEN);
          settings++;
          display_settings();
          break;
        case '2':
          memstore.set_state(static_cast<MFA>(settings), MFA_STATE::_INACTIVE);
          settings++;
          display_settings();
          break;
        case '3':
          handle_settings_modify();
          break;
        default:
          break;
      }
    }
  }
}

void setup() {
  // Servo
  lockServo.attach(SERVO_PIN);

  pinMode(PIR_PIN, INPUT);

  // Display
  lcd.init();
  lcd.backlight();
  init_icons(lcd);

  // Serial
  Serial.begin(115200);

  // Random for OTP
  randomSeed(micros());

  // // RFID
  // SPI.begin();
  // rfid.PCD_Init();

  // Check if pin exists in eeprom
  if (!memstore.has_pin()) {
    while (!set_new_pin());
  }

  // Sync state of lock to eeprom
  currentState = init_state();
  showStartupMessage();

  // Sync the lock to eeprom
  if (memstore.get_door_state()) {
    unlock();
  } else {
    lock();
    display_locked_state();
  }
}

void loop() {
  switch(currentState) {
    case MFA::PIR:
      handle_pir();
      break;
    case MFA::PIN:
      handle_pin();
      break;
    case MFA::OTP:
      handle_otp();
      break;
    case MFA::RFID:
      handle_rfid();
      break;
    case MFA::NONE:
      handle_unlocked();
      break;
  }
}
