# Smart-Door-Lock-MFA-IoT-Boards
Programming for IoT Boards Project
There are two versions of the project:
# Door Lock with MFA and Memory
  https://wokwi.com/projects/439077869366072321
  Has MFA + EEPROM for Permanent Memory
# Door Lock with Blynk Integration
  https://wokwi.com/projects/445698403237792769
  Has MFA + Blynk Integration for sending alert for multiple incorrect attempts.
  NOTE: Since the Blynk version uses ESP32, EEPROM is not supported

# Features
* PIR Sensor for motion detection
* PIN for door lock ( At the beginning, if no PIN in memory, it will ask to create one )
* OTP sent on Serial
* RFID tag ( Due to lack of RFID support in wokwi, The RFID id should be sent on Serial )
* Settings to enable or disable each feature when door is unlocked
* Settings to change pin and add / delete / modify stored RFID ids
* Memory using EEPROM ( Only in First version )
* Wifi and Blynk Integration ( Only in Second version )
* A good looking LCD display
