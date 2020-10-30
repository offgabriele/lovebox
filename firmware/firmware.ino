#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include "version.h"
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>
#include <Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#define FS_NO_GLOBALS
#include <FS.h>

#ifdef ESP32
#include "SPIFFS.h" // Needed for ESP32 only
#endif

#include <SPI.h>

// https://github.com/Bodmer/TFT_eSPI
#include <TFT_eSPI.h>                 // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();            // Invoke custom library
//TFT_eSprite spr = TFT_eSprite(&tft);  // Declare Sprite object "spr" with pointer to "tft" object

// https://github.com/Bodmer/TFT_eFEX
#include <TFT_eFEX.h>              // Include the extension graphics functions library
TFT_eFEX  fex = TFT_eFEX(&tft);    // Create TFT_eFX object "efx" with pointer to "tft" object
#define BLACK 0x0000  // New colour

Servo myservo;

int pos = 90;

int increment = -1;

const char *FIRMWARE_VERSION = _FIRMWARE_VERSION;
//const char FIRMWARE_VERSION[] = ("__LoVeBoX__ " __DATE__ " " __TIME__ "__");

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "it.pool.ntp.org");

StaticJsonDocument<1024> jsonDoc;  // Allocate a 1024-byte buffer for the JSON document.

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
  }
}

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
  Serial.println(payload);
  DeserializationError err = deserializeJson(jsonDoc, payload);
  JsonObject jobj = jsonDoc.as<JsonObject>();
  const char *nextVersion = (const char *)jobj["version"];
  if (strcmp(nextVersion, FIRMWARE_VERSION) &&
      strcmp(nextVersion, "none") &&
      strcmp(nextVersion, "") &&
      strcmp(nextVersion, "current")) {
    Serial.println("OTA: Update required to " + String(nextVersion));
    fex.drawJpgFile(SPIFFS, "/updatelight.jpg", 0, 0);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(3); tft.setTextFont(2);
    uint16_t len = tft.textWidth("Updating...");
    uint16_t pos = 160 - (len / 2);
    tft.setCursor(pos, 170, 2); tft.setTextSize(3); tft.print("Updating...");
    startFirmwareUpdate(nextVersion);
  }
  else {
    Serial.println("OTA: Latest version installed");
  }
}

bool isRead() {
  int rawValue = analogRead(36);
  Serial.println(rawValue);
  if (rawValue > 1000) {
    return true;
  } else {
    return false;
  }
}

void spinServo() {
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}

void showMessage (const char *message) {
  int x, y;
  String s((const __FlashStringHelper*) message);
  tft.fillScreen(BLACK);
  tft.setTextSize(2); tft.setTextFont(2);
  uint16_t len = tft.textWidth(message);
  tft.setCursor(pos, 120, 2); tft.setTextSize(2); tft.print(message);
  while (!isRead()) {
    spinServo();
  }
  HTTPClient http;
  timeClient.update();

  Serial.println(timeClient.getFormattedTime());

  http.begin("https://lovebox-offgabriele.firebaseio.com/messages/latest.json");
  http.addHeader("Content-Type", "application/json");
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  String time_stamp = timeClient.getFormattedTime();
  String payload = "{\"displayed\":\"true\",\"time\":\"";
  payload += time_stamp;
  payload += "\"}";
  Serial.println(payload);
  int httpResponseCode = http.PATCH(payload);
  if (httpResponseCode > 0) {

    String response = http.getString();

    Serial.println(httpResponseCode);
    Serial.println(response);

  } else {

    Serial.print("Error on sending PUT Request: ");
    Serial.println(httpResponseCode);

  }

  http.end();
  while (isRead()) {
    Serial.println("Aspettando");
  }
  fex.drawJpgFile(SPIFFS, "/logo2.jpg", 0, 0);
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
  if (strcmp(nowmessage, "true")) {
    const char *newMessageString = (const char *)jobj["message"];
    showMessage(newMessageString);
  }
}

void configModeCallback(WiFiManager *myWiFiManager) {
  tft.fillScreen(BLACK);
  fex.drawJpgFile(SPIFFS, "/configbackground.jpg", 0, 0);
  tft.setTextSize(3); tft.setTextFont(1);
  uint16_t len = tft.textWidth("Configura Lovebox");
  uint16_t pos = 160 - (len / 2);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(pos, 10, 1); tft.setTextSize(3); tft.print("Configura Lovebox");
  tft.setCursor(0, 50, 2); tft.setTextSize(1.5); tft.println("   1. Connetiti alla rete LOVEBOX.configure");
  tft.println("   2. Visita 192.168.4.1");
  tft.println("   3. Clicca Configura WiFi");
  tft.println("   4. Seleziona la tua rete e \n      inserisci la password");
  tft.println();
  tft.setTextSize(2); tft.setTextFont(1);
  uint16_t len1 = tft.textWidth("Questa finestra si");
  uint16_t pos1 = 160 - (len1 / 2);
  tft.setCursor(pos1, 150, 1); tft.println("Questa finestra si");
  uint16_t len2 = tft.textWidth("chiudera' quando");
  uint16_t pos2 = 160 - (len2 / 2);
  tft.setCursor(pos2, 170, 1); tft.println("chiudera' quando");
  uint16_t len3 = tft.textWidth("Lovebox sara' connessa");
  uint16_t pos3 = 160 - (len3 / 2);
  tft.setCursor(pos3, 190, 1); tft.println("Lovebox sara' connessa!");
}

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

void setup() {
  tft.begin(); tft.setRotation(1); tft.fillScreen(BLACK);
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");
  Serial.println("Lovebox. Running version: " + String(FIRMWARE_VERSION));
  fex.listSPIFFS();
  fex.drawJpgFile(SPIFFS, "/logo2.jpg", 0, 0);
  myservo.attach(19);
  // Initialize the output variables as outputs


  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
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
  timeClient.begin();
  server.begin();
  readConfig();
}

void loop() {


  // Fill screen with grey so we can see the effect of printing with and without
  // a background colour defined
  //isRead();
  spinServo();
  readConfig();
  //newMessage();
  //delay(10000);


}
