# Project_VacTrack
### Introduction
This is the project to track the vaccination carrier with gps coordination along time-stamp. This device currently works on Wifi and GSM module can be added. It stores data on SD card regardless of internet, if device is online it sends data to webserver and stores in sd card, if offline it only stores in sd card.
### Hardware required
+ NodeMCU
+ RTC module
+ SD card module
+ Neo 6m GPS module
+ Charger module
+ 3.7volt rechargeable battery with plastic case
+ PCB borad
+ Female and male headers
+ Female to Female jumpers
### Software required
+ Arduino IDE 
+ libraries required
  - SD
  - RTClib
  - ESP8266WiFi
  - SoftwareSerial
  - TinyGPS++
+ Any webserver. I used AWS for web hosting.
