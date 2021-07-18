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
+ OLED 128x64 display (optional)
### Software required
+ Arduino IDE 
+ libraries required
  - SD
  - RTClib
  - ESP8266
  - SoftwareSerial
  - TinyGPS++
  - Adafruit SSD 1306 and Adafruit gfx (for ver. Oled display only)
+ Any webserver. I used AWS for web hosting.
  - Flask
### Configuration
Download and extract my Flask web application to your required destination. And start flask webapp with any deployment (I used gunisorn which was easy made the process easy) using following intructions from this git https://github.com/app-generator/flask-adminkit.

Device can be wired according to given circuit diagram. Now on Arduino IDE edit the following line to your requirements.
``` // Tracker ID
const char* pgtid = "TID001";```

