#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
//
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// Tracker ID
const char* pgtid = "TID001";

// SD stuff
const int chipSelect = D8; //D8 pin on NodeMCU

// RTC stuff
RTC_DS3231 rtc;

// Wifi stuff
#ifndef STASSID
#define STASSID "your_ssid"
#define STAPSK  "your_passwd"
#endif
const char* ssid     = STASSID;
const char* password = STAPSK;
const char* host = "google.com";
const uint16_t port = 80;

// GPS Stuff
static const int RXPin = 2, TXPin = 16;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

// WEB Server
//const char* host = "";
//const uint16_t port = ;
String serverName = "http://vactrackproject.ddns.net:port/inputdata"; //specifie port
HTTPClient http;

// Variables
String latt;
String lngt;
float longitude;
float latitude;

void setup() {
  Serial.begin(9600);
  // GPS start
  ss.begin(GPSBaud);
  Serial.println();
  // SD start
  Serial.println("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  Serial.println();
  // RTC start
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //Wifi start
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  
}

bool checkGPS() {
  if (gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
    return true;
  } else {
    return false;
  }
}

void onlineUpdate() {
  if (gps.location.isValid()){
    // Get latt, lngt
    latitude = (gps.location.lat());
    longitude = (gps.location.lng());
    latt = String(latitude,6);
    lngt = String(longitude,6);
    
    // Print latt, lngt
    Serial.println(latt);
    Serial.println(lngt);
      
    HTTPClient http;

    String serverPath = serverName +"?pgtid="+pgtid+"&longitude="+lngt+"&latitude="+latt;
      
    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());
      
    // Send HTTP GET request
    int httpResponseCode = http.GET();
      
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    } 
    http.end(); 
    delay(30000);
  } else {
    Serial.println("GPS data not valid");
    //Serial.println(gps.location.lat(), 6);
    //Serial.println(gps.location.lng(), 6);
    //Serial.println(gps.location.isValid());
    delay(1000);
  }
}

void offlineUpdate() {
  DateTime now = rtc.now();
  //GPS data
  if (gps.location.isValid()) {
    // get latt and lngt
    float latitude = (gps.location.lat());
    float longitude = (gps.location.lng());
    latt = String(latitude,6);
    lngt = String(longitude,6);

    // Print latt, lngt
    Serial.println(latt);
    Serial.println(lngt);

    if (now.hour() > 23) {
      return;
    }
      String dataS = String(now.month())+"-"+String(now.day())+"-"+String(now.year())+","+String(now.hour())+":"+String(now.minute())+":"+String(now.seconds())+","+latt+lngt;

      String fname = "data.txt";

      Serial.println("data and filename: "+dataS+","+fname);
      writesd(fname,dataS)
    
    delay(30000);
  } else {
    Serial.println("GPS data not valid");
    //Serial.println(gps.location.lat(), 6);
    //Serial.println(gps.location.lng(), 6);
    //Serial.println(gps.location.isValid());
    delay(1000);
  }
}

void writesd(String filename, String dataString) {
  File dataFile = SD.open(filename, FILE_WRITE);
  // if the file opened okay, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);
  } else {
    Serial.println("error opening "+filename);
  }
}

void loop()
{
  // Check gps module 
  if (checkGPS()== true) {
      delay(5000);
    }
  if (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      Serial.print("connecting to ");
      Serial.print(host);
      Serial.print(':');
      Serial.println(port);
      // Use WiFiClient class to create TCP connections
      WiFiClient client;
      // If NO internet store log to date.txt on SD
      if (!client.connect(host, port)) {
        Serial.println("Offline");
        offlineUpdate();
      } else {
        // If internet update server
        while (client.connected()) {
          Serial.println("Online");
          onlineUpdate();
          //offlineUpdate();  //test on offline fn
        }
      }
      
    }
  }
}
