#include <SD.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET LED_BUILTIN // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();
  
  showLogo();
  // Invert and restore display, pausing in-between
  display.invertDisplay(true);
  delay(2000);
  display.invertDisplay(false);
  delay(2000);
  display.clearDisplay();
  
  // GPS start
  ss.begin(GPSBaud);
  display.setCursor(0,  0);
  printText("Starting on GPS...\n");
  Serial.println();
  Serial.println("Initialization done.");
  printText("Initialization done.\n");
  
  // RTC start
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    printText("Couldn't find RTC\n");
    Serial.flush();
    abort();
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    printText("RTC lost power, let's set the time!\n");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  // SD start
  Serial.println("Initializing SD card...");
  printText("Initializing SD card...\n");
  if (!SD.begin(CS_PIN)) {
    Serial.println("initialization failed!");
    printText("initialization failed!\n");
    return;
  }
  // SD Cheack for file else create
  if(SD.begin(CS_PIN)){
    File file = SD.open("/test.txt");
    if(!file) {
      Serial.println("File doens't exist\nCreating file...");
      printText("File doens't exist\nCreating file...\n");
      txt = "/test.txt";
      d = "Date,Time,latt,lngt \r\n";
      datafn(txt.c_str(),d.c_str());
    } else {
      Serial.println("File already exists");
      printText("File already exists\n");  
    }
    file.close();
  } else {
    Serial.println("SD card not found.!");
    printText("SD card not found.!\n");
  }
  SD.end();
  delay(5000);
  
  //Wifi start
  Serial.print("Connecting to ");
  display.clearDisplay();
  display.setCursor(0,0);
  printText("Connecting to ");
  Serial.println(ssid);
  printText(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    printText(".");
  }
  Serial.println("WiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
  printText("\nWiFi connected\nIP address: \n");
  printText(WiFi.localIP().toString().c_str());
  printText("\n");
  delay(5000);
}

bool checkGPS() {
  if (gps.charsProcessed() < 10) {
    display.clearDisplay();
    display.setCursor(0,0);
    printText("No GPS detected: If initial start wait for 10s else check wiring.");
    Serial.println(F("No GPS detected: If initial start wait for 2min else check wiring."));
    return true;
  } else {
    return false;
  }
}

void onlineUpdate() {
  //Get current time
  DateTime now = rtc.now();
  display.clearDisplay();
  display.setCursor(0,0);
  String tm = "Sat:"+ String(gps.satellites.value())+" "+String(now.hour())+":"+String(now.minute())+":"+String(now.second())+" Online";
  printText(tm.c_str());
  //GPS data
  if (gps.location.isValid()){  
    latt = String(float(gps.location.lat()),6);
    lngt = String(float(gps.location.lng()),6);
    
    HTTPClient http;
    serverPath = serverName +"?pgtid="+pgtid+"&longitude="+lngt+"&latitude="+latt;
      
    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());
      
    // Send HTTP GET request
    int httpResponseCode = http.GET();
      
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: "+httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("Error code: "+httpResponseCode);
    } 
    d = String(now.month())+"-"+String(now.day())+"-"+String(now.year())+","+String(now.hour())+":"+String(now.minute())+":"+String(now.second())+","+latt+","+lngt+","+String(httpResponseCode)+"\r\n";
    txt = String(now.month())+String(now.day())+String(now.year())+".txt";
    String tm = "\nLatitude: "+latt+"\nLongitude"+lngt+"\nResponseCode: "+String(httpResponseCode)+"\n";
    printText(tm.c_str());
    datafn(txt.c_str(),d.c_str());
    http.end(); 
  } else {
    Serial.print("GPS data not valid: "+String(gps.location.lat(), 6)+" "+String(gps.location.lng(), 6)+" "+String(gps.location.isValid())+"\n");
    String tm="\nGPS data not valid\nValues for Debugging\nLatitude: "+String(gps.location.lat())+"\nLongitude: "+String(gps.location.lng())+"\nValid: "+String(gps.location.isValid())+"\n";
    printText(tm.c_str());
  }
}

void offlineUpdate() {
  //Get current time
  DateTime now = rtc.now();
  display.clearDisplay();
  display.setCursor(0,0);
  String tm = "Sat:"+ String(gps.satellites.value())+" "+String(now.hour())+":"+String(now.minute())+":"+String(now.second())+" Offline";
  //GPS data
  if (gps.location.isValid()) {
    // get latt and lngt
    latt = String(float(gps.location.lat()),6);
    lngt = String(float(gps.location.lng()),6);

    d = String(now.month())+"-"+String(now.day())+"-"+String(now.year())+","+String(now.hour())+":"+String(now.minute())+":"+String(now.second())+","+latt +","+lngt+"\r\n";
    txt = String(now.month())+String(now.day())+String(now.year())+".txt";
    Serial.println("data: "+d+" and filename: "+txt);
    datafn(txt.c_str(),d.c_str());
    String tm = "\nLatitude: "+latt+"\nLongitude"+lngt+"\n";
    printText(tm.c_str());
  } else {
    Serial.print("GPS data not valid: "+String(gps.location.lat(), 6)+" "+String(gps.location.lng(), 6)+" "+String(gps.location.isValid())+"\n");
    String tm="\nGPS data not valid\nValues for Debugging\nLatitude: "+String(gps.location.lat())+"\nLongitude: "+String(gps.location.lng())+"\nValid: "+String(gps.location.isValid())+"\n";
    printText(tm.c_str());
  }
}


void loop()
{
  if (checkGPS()== true) {
      delay(10000);
    }
  if (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      Serial.print("connecting to "+String(host)+":"+String(port)+"\n");
      // Use WiFiClient class to create TCP connections
      WiFiClient client;
      // If NO internet store log to date.txt on SD 
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
      printText("\nFile written");
    } else {
      Serial.println("Write failed");
      printText("\nWrite failed");
    }
    dataFile.flush();
    dataFile.close(); 
  } else {
    Serial.println("SD card not found.!");
  }
  SD.end();
}

void showLogo(void) {
  display.setCursor(0,0);
  display.setTextSize(2);
  display.setTextColor (WHITE);
  display.print("\n Project\n VacTrack");
  display.display();
}

void printText(const char* txt){
  display.setTextSize(1);
  display.setTextColor (WHITE);
  display.print(txt);
  display.display();
}
