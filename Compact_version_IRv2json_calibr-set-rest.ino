#include <ESP8266HTTPClient.h> //For API get?
#include "OneButton.h"  //For calibration button, checkin and checkout
#include <NTPClient.h> //For getting the current date
#include <WiFiUdp.h>  //For getting the current date
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <FS.h>   //For the file system, needed to access html and js
#include <ArduinoJson.h>
#include <Wire.h>   //IR   SCL=D1 SDA=D2
#include "vl53l1_api.h"  //IR

//To Add the JSON buffers where they bellong, separate per each type of message
const size_t bufferSize = JSON_OBJECT_SIZE(7); //JSON
DynamicJsonBuffer jsonBuffer(bufferSize); //JSON

const size_t bufferSize2 = JSON_ARRAY_SIZE(30) + 30*JSON_OBJECT_SIZE(7);
DynamicJsonBuffer arrayBuffer(bufferSize2);

JsonObject& root = jsonBuffer.createObject();
JsonObject& root2 = jsonBuffer.createObject();
JsonObject& merged = jsonBuffer.createObject();
//JsonArray& arr = jsonBuffer.createArray();
JsonArray& arr = arrayBuffer.createArray();
char bufferArray[4096];  //This one is needed to be high enough so that all of the array will be shown and send to the API. Currently around 20+ reps

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String formattedDate,dayStamp;
//String dayStamp;

VL53L1_Dev_t                   dev;
VL53L1_DEV                     Dev = &dev;
//int  status, distanceMax,distanceMin;

OneButton button(0, true);
unsigned long time_since_last_reset=0;  //For the 3 sec calib measurement


//const char *ssid = "SFR-3ab0";   // Update to contain your network information
//const char *password = "QRDPNFWWCU4A";
String account = "dummy";
String url = "http://192.168.0.13:8080";
int rfid = 12345;

long duration;
int interval=10;
int weight=80;
unsigned long previousMillis,endRest,currentMillis,down;
int distanceIR,n,s,target,calibr,flag,restPosition,xtend,contract,machine,distanceCm,powerContract,powerXtend,status, distanceMax,distanceMin ; // distanceInch;


const char *ssid = "AndroidAP";
const char *password = "f29946d90373";

WebSocketsServer webSocket = WebSocketsServer(81);   // Initialise websockets, web server
ESP8266WebServer server(80);

void setup() {
pinMode(LED_BUILTIN, OUTPUT); //For the LED blinking
button.attachDoubleClick(doubleclick); 
button.attachClick(click1);
button.attachLongPressStart(longPressStart1); 

uint8_t byteData;  //IR
uint16_t wordData;  //IR
Wire.begin(); //IR
Wire.setClock(400000);

restPosition=85;     //the lowest position far from the sensor
machine=4;           //Change once
target=99;          //Depends on the trainee Number of reps
calibr=20;         // Next to the sensor; To count 1 rep; Depends on the trainee ?? Needs to be verified in the gym
//int weight=80;    //Write the weight of the exercise used
s=1;   //Here is to take this from the REST API every time RFID is checkedin

//pinMode(LED_BUILTIN, OUTPUT);  //For the buildin led to verify if the card is read

Serial.begin(115200);

//IR
Dev->I2cDevAddr = 0x52;
  VL53L1_software_reset(Dev);
  VL53L1_RdByte(Dev, 0x010F, &byteData);
  VL53L1_RdByte(Dev, 0x0110, &byteData);
  VL53L1_RdWord(Dev, 0x010F, &wordData);
  status = VL53L1_WaitDeviceBooted(Dev);
  status = VL53L1_DataInit(Dev);
  status = VL53L1_StaticInit(Dev);
  status = VL53L1_SetDistanceMode(Dev, VL53L1_DISTANCEMODE_SHORT);
  status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(Dev, 50000);
  status = VL53L1_SetInterMeasurementPeriodMilliSeconds(Dev, 50); // reduced to 50 ms from 500 ms in ST example
  status = VL53L1_StartMeasurement(Dev);
//IR

Serial.print("Configuring wifi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  IPAddress myIP = WiFi.localIP();
  Serial.println(myIP);    // Print out the IP address

timeClient.begin(); // To get the time
  
  SPIFFS.begin();    // Begin access to our file system (which stores the HTML)
  // Start the websockets and link the events function
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  // Configure web server to host HTML files
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
  server.begin();
  getDate(); //To get the current date
}
void loop() {
button.tick(); 
//Serial.println("Distance Max:");  //Debug
  //Serial.println(restPosition);   //Debug
  //Serial.println("Distance Min:");   //Debug
  //Serial.println(distanceMin);   //Debug

//IR
static VL53L1_RangingMeasurementData_t RangingData;
  status = VL53L1_WaitMeasurementDataReady(Dev);
  if(!status){
    status = VL53L1_GetRangingMeasurementData(Dev, &RangingData);
    distanceIR =RangingData.RangeMilliMeter/10;
    if(status==0){
      Serial.println(distanceIR);}  //DEBUG
    status = VL53L1_ClearInterruptAndStartMeasurement(Dev);
  }

if(distanceIR<calibr) { //Executed properly exercise
  if(flag==0)  {
  flag=1;
  n++;
  
currentMillis = millis();   //Contraction time
if(currentMillis - previousMillis > interval) {
  //contract= (currentMillis - previousMillis);     //Change here the previousMillis with up-to-date-one
    contract= (currentMillis - down)/2;  
 endRest = currentMillis;
 powerContract = ((weight*9.81)*(restPosition-calibr))/contract;  //in Watt

//JSON
char buffer[256];
root["reps"] = n;
root["power"] = powerContract;
root["weight"] = weight;
root["set"] = s;
root["contract"] = contract;
root["account"] = account;
root["machine"] = machine;

merged["reps"] = n;
merged["power"] = powerContract;
merged["weight"] = weight;
merged["sets"] = s;                               //Rename set to sets in the dashboard too, have it consistent
merged["contraction"] = contract;
merged["account"] = account;
merged["machine"] = machine;
merged["date"] = dayStamp;

root.printTo(buffer);
 
 Serial.println(buffer);   //Debug
 webSocket.broadcastTXT(buffer);

  }
 }
}

if(distanceIR>restPosition) {
   if(previousMillis==0){
 unsigned long currentMillis = millis();
 previousMillis = currentMillis;  //try to put the previousMillis above in 1 line
   }
  if(flag==1) {

previousMillis = millis();  //Xtend time
  //if(currentMillis - previousMillis > interval) {
 xtend= ( previousMillis-endRest); //Check this one here if assignment is needed
 //powerXtend = ((weight*9.81)*(restPosition-calibr))/xtend;   //in Watt
 flag=0;

char buffer2[90];
char buff_merged[256];
root2["extension"] = xtend;
merged["extension"] = xtend;
root2.printTo(buffer2);
merged.printTo(buff_merged);
//Serial.println(buffer2);  //Debug
webSocket.broadcastTXT(buffer2);

//Combine root and root 2 into merged
//Currently repeating the merged without the old values

//arr.add(merged);   //take and repeat the last value of extension in all of the array
arr.add(buff_merged);
arr.printTo(bufferArray);
//arr.printTo(Serial);
//Serial.println(bufferArray);  //Find a way to remove the escaping from the list
}
else if (flag==0){ //Start Resting
flag=5; ///Change the flag so it will run once
}
down=currentMillis;
}

if( distanceIR < restPosition){  //Stop Resting
  unsigned long endRest=millis(); 
  if(flag==5){ 

//if((endRest-previousMillis)>5000) { //Consider only rest higher than 5 secs LOGOUT
 //Serial.println("LOGOUT!"); 
  //LOGOUT
  //}
//Serial.println(RestContainer); //Cummulative rest up until now
//Serial.println((endRest-previousMillis)/1000.0,2);  //This is the current rest only
 previousMillis=endRest;
flag=0; //change the flag so it will run once
  }
}

if (n==target){      //once you finish the set, logout and sent the JSONS to REST API
Serial.println("LOGOUT and PUSH TO API the current progress");
  //webSocket.broadcastTXT(buffer3);
  n=0;
  }

webSocket.loop();
  server.handleClient();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch(type) {
    case WStype_DISCONNECTED: {      // Runs when a user disconnects
      Serial.printf("User #%u - Disconnected!\n", num);
      break;
    }  
    case WStype_CONNECTED: {      // Runs when a user connects
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("--- Connection. IP: %d.%d.%d.%d Namespace: %s UserID: %u\n", ip[0], ip[1], ip[2], ip[3], payload, num);
      break;
    }
    case WStype_TEXT: {      // Runs when a user sends us a message
      String incoming = "";
      for (int i = 0; i < lenght; i++) {
        incoming.concat((char)payload[i]);
      }
      int deg = incoming.toInt();

if (deg==0) {
calibration();
}
else if (deg >= 1 && deg <=5) {
  machine=deg;
}
else if (deg==999){
logout();  
}
else{
 weight=deg;
}
      Serial.println(deg); 
      break;
    }   
  }
}

// A function we use to get the content type for our HTTP responses
String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

// Takes a URL (for example /index.html) and looks up the file in our file system,
// Then sends it off via the HTTP server!
bool handleFileRead(String path){
  #ifdef DEBUG
    Serial.println("handleFileRead: " + path);
  #endif
  if(path.endsWith("/")) path += "index.html";
  if(SPIFFS.exists(path)){
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, getContentType(path));
    file.close();
    return true;
  }
  return false;
}

String put_calibration(String account=account, int machine=machine, int restposition=restPosition, int calibr=calibr) {
const size_t bufferSize = JSON_OBJECT_SIZE(4) + 150;
  DynamicJsonBuffer calibr_buffer(bufferSize);
  JsonObject& calibrat = calibr_buffer.createObject();
  char buffer[256];
  calibrat["account"] = account;
  calibrat["machine"] = machine;
  calibrat["restposition"] = restposition;  
  calibrat["calibr"] = calibr;            
  calibrat.printTo(buffer);
Serial.println(buffer);  //DEBUG
return buffer;
}

String POST_API(String url, String JSON="0"){
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin(url);  //Specify request destination      
   int httpCode = http.POST(JSON);   //Send the request

  String payload = http.getString();  //Get the answer of the request
  Serial.println(payload);  //DEBUG
    http.end();   //Close connection 
    return payload;  
  }
}

void doubleclick() {    //CALIBRATION MODE
calibration();
}

String GET_API(String url, String JSON="0"){
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin(url);  //Specify request destination      
   int httpCode = http.GET();   //Send the request

String payload = http.getString();  //Get the answer of the request
Serial.println(payload);  //DEBUG
    http.end();   //Close connection 
    return payload;  
  }
}

void click1() {
  //Serial.println("Button 1 click.");
  login();
}

void longPressStart1() {
  Serial.println("Button longPress start");
  logout();
}

void login(){
  Serial.println("Get date");
  getDate();
  Serial.println("Get account and calibr");
//  POST_API(url+ "/checkin/" + rfid + "/" + machine);  //TO select the account and calibration values

//New
StaticJsonBuffer<200> tempBuffer;
JsonObject& temp = tempBuffer.parseObject(POST_API(url + "/checkin/"+ rfid + "/"+ machine));
account = temp["account"].asString();
s = temp["sets"];
calibr= temp["calibr"];
restPosition= temp["restposition"];

//Wait for the training to start
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off
  delay(5000);                       // wait 5 seconds
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED on
  Serial.println("Create buffers");
  n=0;   // reset the n
//Start time measurement
unsigned long down = millis();
}

void logout(){
  Serial.println("Logging out");
  Serial.println("Post checkout");
  Serial.println("Post training details");
POST_API(url+ "/checkout/" + account + "/" + machine, bufferArray);  //Send calibration values to DB
  Serial.println("Clear buffers");
  arrayBuffer.clear();
  JsonArray& arr = arrayBuffer.createArray();
  account="dummy";
}

String getDate(){

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }

  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.println(dayStamp);
  return dayStamp ;
}

void calibration() {    //CALIBRATION MODE
 int interval_one=3000;   //measure top for 3 secs
 int interval_two=3000;   //measure bottom for 3 secs
 int distanceNow, distanceMax, Sum;
  // what happens when button is double-clicked
Serial.println("Double Click");
digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off
  delay(5000);                       // wait 5 seconds before start measuring
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED on

time_since_last_reset=millis();
while((millis()-time_since_last_reset)< interval_one) {
  Serial.println("Measuring max distance:");
  yield();

//Start first measurement for Max
 static VL53L1_RangingMeasurementData_t RangingData; 
  status = VL53L1_WaitMeasurementDataReady(Dev);
  if(!status)
  {
    status = VL53L1_GetRangingMeasurementData(Dev, &RangingData);
    if(status==0)
{
    distanceNow =RangingData.RangeMilliMeter/10;
    delay(200);  
    Sum =Sum +distanceNow;
 Serial.println(distanceNow);
 Serial.println(Sum); 
}
}
}
 distanceMax=Sum/15;
 Serial.println("Average Distance Max:");
 Serial.println(distanceMax);
 restPosition=distanceMax;
 Sum=0;
 Serial.println(Sum); 

//Start second measurement for Min
digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off
  delay(5000);                       // wait 3 seconds before start measuring
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED on

time_since_last_reset=millis();
while((millis()-time_since_last_reset)< interval_two) {
  Serial.println("Measuring min distance:");
  yield();
  //Start first measurement for Min
 static VL53L1_RangingMeasurementData_t RangingData; 
  status = VL53L1_WaitMeasurementDataReady(Dev);
  if(!status)
  {
    status = VL53L1_GetRangingMeasurementData(Dev, &RangingData);
    if(status==0)
{
    distanceNow =round(RangingData.RangeMilliMeter/10);
    delay(200);
    Sum =Sum +distanceNow;
 Serial.println(distanceNow);
 Serial.println(Sum); 
}
}
}
distanceMin=round(Sum/15);
 Serial.println("Average Distance Min:");
  Serial.println(distanceMin);
calibr= distanceMin;
Sum=0;

//Send a confirmation via WS that the calibration is over
webSocket.broadcastTXT("Calibration completed!");
POST_API(url+ "/calibration", put_calibration());  //Send calibration values to DB
}
