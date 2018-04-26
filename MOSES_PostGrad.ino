
/* I used code from
 *  https://randomnerdtutorials.com/esp8266-weather-forecaster/ 
 *  So there were some variables he used that I couldn't tell their function.
 * 
 * */
/*********************************************************************
This is an example sketch for our Monochrome Nokia 5110 LCD Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338

These displays use SPI to communicate, 4 or 5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <ArduinoJson.h>
#include <sim800l.h> 
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#define JSON_BUFF_DIMENSION 2500
#define CLR display.clearDisplay()
#define SEND(X) display.println(X)
#define SHOW display.display()
#define WAIT(X) delay(X)
#define SENSE 3
#define PUMP_ON digitalWrite(13,HIGH)
#define PUMP_OFF digitalWrite(13,LOW)
const char *request = "http://api.openweathermap.org/data/2.5/forecast?q=Akure,NG&APPID="
"<YOUR_API_KEY_HERE>&mode=json&units=metric&cnt=2";
const char *apn = "etisalat.com.ng";
const char uname[] = "";
const char pword[] = "";
const unsigned int port = 80;

char buffer[512]; //have mo idea what this does

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

//some Emmanuel Odunlade stuff here.
unsigned int jsonend = 0;
boolean startJson = false;
boolean pump_is_active = false;
String weatherNow;
String weatherLater;
const unsigned long int postInterval = 10 * 60 * 1000; //10 mins
unsigned long int lastConnectionTime = postInterval; //This would probable change every 10 mins
String server_response;

//always enter the number with the country code.
char phone_number[] = "+2348095927348";

void parseJson(const char* jsonString);
void MakeDecisionToPump(String, String, String);
void sendRequest(void);
void receiveJsonData(void);

NETWORK gsm;

void setup() {
  Serial.begin(9600);
  display.begin();
  display.setContrast(50);
  display.setTextSize(1);
  SEND(F("Setting up network..."));
  SHOW;
  WAIT(1000);
  server_response.reserve(JSON_BUFF_DIMENSION);
}

void loop() {
  //openweathermonitor.org requires 10mins between request intervals.
  //check if 10mins has passed then connect again and make request.
  if (millis() - lastConnectionTime >= postInterval) {
    lastConnectionTime = millis();
    gsm.setupInternet(apn, request);
    gsm.sendRequest();
    SEND(F("Request sent.")); SHOW; WAIT(1000); CLR;
    SEND(F("Waiting for server...")); SHOW; WAIT(1000); CLR;

    if(gsm.waitForServerResponse() != 0){
      receiveJsonData();
      MakeDecisionToPump(weatherNow, weatherLater, "rain");
    }   
  }
}

void receiveJsonData(void) 
{
  char c;
  while(1) {
    c = gsm.readResponse();
    if(c == '{') {
      startJson = true;
      jsonend++;
    }
    if (c == '}') {
      jsonend--;
    }
    if (startJson == true) {
      server_response += c;
    }
    if (jsonend==0 && startJson==true) {
      parseJson(server_response.c_str());
      break;
    }
  }
}


void MakeDecisionToPump(String nowT, String later, String weatherType) {
  int indexNow = nowT.indexOf(weatherType);
  int indexLater = later.indexOf(weatherType);
  unsigned int value = 0;
  //turn moisture sensor on here.
  for(int i=0;i<30;i++){
    value = analogRead(SENSE);
    value = map(value,0,1024,0,100);
  }
  //if it's so not gonna rain and the ground is dry.
  if ( (weatherType != "rain") && value<30){
    pump_is_active = true;

    //this method requires you add '\r' to your message.
    if(!(gsm.initAndSendSMS("Pump is active.\r", "+2348142357637"))){
      SEND(F("Message sending failed.")); SHOW; WAIT(2000); CLR;
    }
    do {
      PUMP_ON;
      delay(20000);
      PUMP_OFF;
      delay(20000); //to allow water to settle
      unsigned int i;
      for(i=0;i<340;i++){
        value = analogRead(SENSE);//screen should show when it's reading.
      }
      value = map(value,0,1024,0,100);
    } while (value<=30);
    pump_is_active = false;
  }
}
      

//EmmaOdun's code here
void parseJson(const char* jsonString) {
  //StaticJsonBuffer<4000> jsonBuffer
  const size_t bufferSize = 2*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + 
  4*JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(2) + 
  3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 
  2*JSON_OBJECT_SIZE(7) + 2*JSON_OBJECT_SIZE(8) + 720;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  //Find fields in JSON tree.
  JsonObject& root = jsonBuffer.parseObject(jsonString);
  if (!root.success()) {
    //success indicator
    return;
  }
  JsonArray& list = root["list"];
  JsonObject& nowT = list[0];
  JsonObject& later = list[1];
  String weatherNow = nowT["weather"][0]["description"];
  String weatherLater = later["weather"][0]["description"];
}

