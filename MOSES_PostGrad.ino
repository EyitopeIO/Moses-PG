/* I used code from
 *  https://randomnerdtutorials.com/esp8266-weather-forecaster/ 
 *  So there were some variables he used that I couldn't tell their function.
 * 
 * */


#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#define Rx_pin 0 //both for GSM module
#define Tx_pin 2
#define JSON_BUFF_DIMENSION 2500

//Emmanuel Odunlade stuff below
String text; 
int jsonend = 0;
boolean startJson = false;
//int status = WL_IDLE_STATUS;

float tempNow;
float humidityNow;
String weatherNow;
float tempLater;
float humidityLater;
String weatherLater;
String RESPONSE; //response from gsm module or server.
String ACCESS_POINT = ""; //for whatever simcard is used.
String IP_ADDRESS; //to be printed on screen.
const unsigned long postInterval = 10 * 60 * 1000; //10 mins
unsigned long lastConnectionTime = postInterval; //This would probable change every 10 mins



SoftwareSerial SIMCARD(Rx_pin, Tx_pin); //serial object to communicate with GSM

char* URL = "http://api.openweathermap.org/data/2.5/forecast?q=Akure,NG&APPID="
"400fa291bf97f191c1efb1413430f2d9" //Application ID
"&mode=json&units=metric&cnt=2";



void setup() {
  //startup indication code here
  text.reserve(JSON_BUFF_DIMENSION);
  
  GSM_INIT(); //GPRS init & SMS init
  //Real time clock init still under consideration though.
 
}

void loop() {
  if (millis() - lastConnectionTime > postInterval) {
    lastConnectionTime = millis();
    //make http request here.
  }
}

/* Always wait for response from last command before next.
 * This is the request logic.
 * 1. Stop all connections.
 * 2. Send HTTP PUT request.
 * 3. Listen for answer. 
 * 4. If no answer in 10 mins, send another.
 * 5. If answer, begin parsing the json data.
 * 
 */

void HttpRequest() {
  SIMCARD.println("AT+HTTPACTION=0"); //send the request to fetch. A network error
  // could occur here and give a 601 code. You could make it retry here...just saying.
  delay(100);
  RESPONSE = SIMCARD.println("AT+HTTPREAD") //command to get online request
  //do something with the data here.
}



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

  //Temperature and humidity
  float tempNow = nowT["main"]["temp"];
  float humidityNow = nowT["main"]["humidity"];
  String weatherNow = nowT["weather"][0]["description"];
  float tempLater = later["main"]["temp"];
  float humidityLater = later["main"]["humidity"];
  String weatherLater = later["weather"][0]["description"];
  
}

void GSM_INIT () {
  //you must wait for module to respond before sending another command.
  //if i don't check response, it's probably not important.
  //make LCD show that it's starting here.
  SIMCARD.begin(4800); //Saw a code online that used 4800 to communicate. Free me.
  SIMCARD.println("AT"); //ping it: "Hey, you there?"
  delay(100); //module should have replied.
  SIMCARD.println("AT+SAPBR=3,1,Contype,GPRS"); //set connection type to GPRS
  delay(100);
  SIMCARD.println("AT+SAPBR=2,1"); //connection successful? check the returned data to know.
  if (SIMCARD.available()) {
    RESPONSE = SIMCARD.readString(); //keep ip address in ram or whatever.
    //print 'connected' on the display here. check if 'OK' in reply to know if
    //successful.
  } 
  SIMCARD.println("AT+SAPBR=3,1,APN," + ACCESS_POINT);
  delay(100);
  SIMCARD.println("AT+SAPBR=1,1"); //enable the GPRS.
  SIMCARD.println("AT+SAPBR=1,1"); //see if ip address in here.
  SIMCARD.println("AT+HTTPINIT"); //enable the HTTP module.
  SIMCARD.println("AT+HTTPPARA=\"CID\",1"); //set ip HTTP bearer profile identifier.
  delay(100);
  SIMCARD.println("AT+HTTPPARA=URL," + URL); //set the url to connect to. obviously...
}

