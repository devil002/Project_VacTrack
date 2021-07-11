#include <SD.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// Tracker ID
const char* pgtid = "TID001";

// SD set pin
const int CS_PIN = 15;  // D8 pin

// RTC set pin
RTC_DS3231 rtc;

// Wifi set SSID and PASSWD
#ifndef STASSID
#define STASSID "Your_SSID"
#define STAPSK  "Your_PASSWD"
#endif
const char* ssid     = STASSID;
const char* password = STAPSK;
const char* host = "google.com";
const uint16_t port = 80;

// GPS set pin
static const int RXPin = 2, TXPin = 16;   //  RX = D4, TX = D0
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

// WEB Server Address
//      replace yourwebaddress:port with your address and port number
String serverName = "http://yourwebaddress:port/inputdata";
HTTPClient http;

//Global variables
String latt;
String lngt;
String serverPath;
String d;
String txt;

void setup() {
  Serial.begin(9600);
  // GPS start
  ss.begin(GPSBaud);
  Serial.println();
  // SD start
  Serial.println("Initializing SD card...");
  if (!SD.begin(CS_PIN)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
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
  // SD Cheack for file else create 
  File file = SD.open("/test.txt");
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    txt = "/test.txt";
    d = "Date,Time,latt,lngt \r\n";
    datafn(txt.c_str(),d.c_str());
  } else {
    Serial.println("File already exists");  
  }
  file.close();
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
  //Get current time
  DateTime now = rtc.now();
  //GPS data
  if (gps.location.isValid()){  
    latt = String(float(gps.location.lat()),6);
    lngt = String(float(gps.location.lng()),6);
    //Serial.println("latt: "+latt+"& lngt: "+lngt);
      
    HTTPClient http;
    serverPath = serverName +"?pgtid="+pgtid+"&longitude="+lngt+"&latitude="+latt;
      
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
    d = String(now.month())+"-"+String(now.day())+"-"+String(now.year())+","+String(now.hour())+":"+String(now.minute())+":"+String(now.second())+","+latt+","+lngt+","+String(httpResponseCode)+"\r\n";
    txt = String(now.month())+String(now.day())+String(now.year())+".txt";
    datafn(txt.c_str(),d.c_str());
    http.end(); 
  } else {
    Serial.print("GPS data not valid: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(" ");
    Serial.print(gps.location.lng(), 6);
    Serial.print(" ");
    Serial.println(gps.location.isValid());
  }
}

void offlineUpdate() {
  //Get current time
  DateTime now = rtc.now();
  //GPS data
  if (gps.location.isValid()) {
    // get latt and lngt
    latt = String(float(gps.location.lat()),6);
    lngt = String(float(gps.location.lng()),6);

    d = String(now.month())+"-"+String(now.day())+"-"+String(now.year())+","+String(now.hour())+":"+String(now.minute())+":"+String(now.second())+","+latt +","+lngt+"\r\n";
    txt = String(now.month())+String(now.day())+String(now.year())+".txt";
    
    Serial.println("data: "+d+" and filename: "+txt);
    datafn(txt.c_str(),d.c_str());
  } else {
    Serial.print("GPS data not valid: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(" ");
    Serial.print(gps.location.lng(), 6);
    Serial.print(" ");
    Serial.println(gps.location.isValid());
  }
}


void loop()
{
  if (checkGPS()== true) {
      delay(10000);
    }
  if (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      Serial.print("connecting to ");
      Serial.print(host);
      Serial.print(":");
      Serial.println(port);
      // Use WiFiClient class to create TCP connections
      WiFiClient client;
      // If NO internet store log to date.txt on SD !client.connect(host, port)
      if (!client.connect(host,port)) {
        Serial.println("Offline");
        offlineUpdate();
        delay(30000);
      } else {
        // If internet update server client.connected()
        while (client.connected()) {
          Serial.println("Online");
          onlineUpdate();
          //offlineUpdate();  // for test purpose only
          delay(30000);
        }
      }
    }
  }
}

void datafn(const char* path, const char* message){
  if(SD.begin(CS_PIN)){
    File dataFile = SD.open(path, FILE_WRITE);

    // if the file is available, write to it:
    if(!dataFile) {
      Serial.println("Failed to open file for writing");
      return;
    }
    dataFile.seek(EOF);
    if(dataFile.write(message)) {
      Serial.println("File written");
      Serial.print("echo ");
      Serial.print(message);
      Serial.print(" >> ");
      Serial.println(path);
    } else {
      Serial.println("Write failed");
    }
    dataFile.flush();
    dataFile.close(); 
  } else {
    Serial.println("SD card not found.!");
  }
  SD.end();
}
