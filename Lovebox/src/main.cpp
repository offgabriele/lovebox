#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include "version.h"

const char *FIRMWARE_VERSION = _FIRMWARE_VERSION;


// Allocate a 1024-byte buffer for the JSON document.
StaticJsonDocument<1024> jsonDoc;
HTTPClient http;
void updateFirmware(String firmwareUrl) {
  WiFiClientSecure client;
  //client.setCACert(rootCACertificate);
  client.setTimeout(12000 / 1000);
  t_httpUpdate_return ret = httpUpdate.update(client, firmwareUrl);
    // Or:
    //t_httpUpdate_return ret = httpUpdate.update(client, "server", 443, "/file.bin");
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
}}

void startFirmwareUpdate(String firmwareVersion) {  
  String url = "https://lovebox-offgabriele.firebaseio.com/firmware/" +
     firmwareVersion + ".json";
  //Serial.println(url);
  http.setTimeout(1000);
  http.begin(url);
  int status = http.GET();
  if (status <= 0) {
    Serial.printf("HTTP error: %s\n", 
        http.errorToString(status).c_str());
    return;
  }
  String payload = http.getString();
  DeserializationError err = deserializeJson(jsonDoc, payload);      
  JsonObject jobj = jsonDoc.as<JsonObject>();
  String firmwareUrl = (const String &)jobj["url"];
  http.end();
  updateFirmware(firmwareUrl);
}

void readConfig() {
 String url = "https://lovebox-offgabriele.firebaseio.com/config/" +
    WiFi.macAddress() + ".json";
  //Serial.println(url);
 http.setTimeout(100000);
 http.begin(url);
 int status = http.GET();
 if (status <= 0) {
   Serial.printf("HTTP error: %s\n", 
       http.errorToString(status).c_str());
   return;
 }
 String payload = http.getString();
 //Serial.println(payload);
 DeserializationError err = deserializeJson(jsonDoc, payload);      
 JsonObject jobj = jsonDoc.as<JsonObject>();
 const char *nextVersion = (const char *)jobj["version"];
 if (strcmp(nextVersion, FIRMWARE_VERSION) &&          
     strcmp(nextVersion, "none") &&        
     strcmp(nextVersion, "") &&        
     strcmp(nextVersion, "current")) {      
   Serial.println("OTA: Update required to " + String(nextVersion));
   startFirmwareUpdate(nextVersion);
 }
 else {
  Serial.println("OTA: Latest version installed");
}
}

void showMessage (const char * message) {
  Serial.println(message);
  HTTPClient http;   
 
   http.begin("https://lovebox-offgabriele.firebaseio.com/messages/latest.json");
   http.addHeader("Content-Type", "application/json");       

   String json_string = "{'displayed':'true'}";
   int httpResponseCode = http.PATCH("{\"displayed\":\"true\"}");   
 
   if(httpResponseCode>0){
 
    String response = http.getString();   
 
    Serial.println(httpResponseCode);
    Serial.println(response);          
 
   }else{
 
    Serial.print("Error on sending PUT Request: ");
    Serial.println(httpResponseCode);
 
   }
 
   http.end();
}

void newMessage() {
  String url = "https://lovebox-offgabriele.firebaseio.com/messages/latest.json";
  Serial.println(url);
  http.setTimeout(100000);
  http.begin(url);
  int status = http.GET();
  if (status <= 0) {
   Serial.printf("HTTP error: %s\n", 
       http.errorToString(status).c_str());
   return;
  }
  String payload = http.getString();
  DeserializationError err = deserializeJson(jsonDoc, payload);      
  JsonObject jobj = jsonDoc.as<JsonObject>();
  const char *nowmessage = (const char *)jobj["displayed"];
  Serial.println(nowmessage);
  if(strcmp(nowmessage, "true")) {
    const char *newMessageString = (const char *)jobj["message"];
    showMessage(newMessageString);
  }
}


// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";

// Assign output variables to GPIO pins
const int output5 = 5;
const int output4 = 4;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("Lovebox. Running version: " + String(FIRMWARE_VERSION));
  // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();
  
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("LOVEBOX.configure");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  server.begin();
  readConfig();
}

void loop() {
 

  Serial.println("Aggiornato");
  readConfig();
  newMessage();
  delay(10000);
 

}