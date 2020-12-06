#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include "version.h"
#include <Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#define FS_NO_GLOBALS
#include <FS.h>
//#include "Web_Fetch.h"
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
  putData("light");
  if (rawValue > 800) {
    return true;
  } else {
    return false;
  }
}

bool isClose(int var) {
  int rawValue = analogRead(36);
  Serial.println(rawValue);
  if (rawValue > (var - 50) && rawValue < (var + 50)) {
    return true;
  } else {
    return false;
  }
}

void spinServo() {
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(5);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(5);                       // waits 15ms for the servo to reach the position
  }
}

String stamp() {
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  String time_stamp = timeClient.getFormattedDate();
  time_stamp.replace("-", "/");
  time_stamp.replace("T", " ");
  time_stamp.replace("Z", " ");
  return time_stamp;
}

void putData(String type) {
  HTTPClient http;
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  http.addHeader("Content-Type", "application/json");
  String payload;
  String url;
  if (type == "display") {
    payload = "{\"displayed\":\"true\",\"timedisplayed\":\"";
    url = "https://lovebox-offgabriele.firebaseio.com/messages/latest.json";
  } else if (type == "read") {
    payload = "{\"read\":\"true\",\"timeread\":\"";
    url = "https://lovebox-offgabriele.firebaseio.com/messages/latest.json";
  } else if (type == "version") {
    payload = "{\"installedversion\":\"";
    payload += FIRMWARE_VERSION;
    payload += ",\"timecheck\":\"";
    url = "https://lovebox-offgabriele.firebaseio.com/config/" +
          WiFi.macAddress() + ".json";
  } else if (type == "light") {
    payload = "{\"lightvalue\":\"";
    payload += analogRead(36);
    payload += "\",\"timecheck\":\"";
    url = "https://lovebox-offgabriele.firebaseio.com/config/" +
          WiFi.macAddress() + ".json";
  }
  payload += stamp();
  payload += "\"}";
  Serial.println(payload);
  http.begin(url);
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
}


void showMessage (const char *message) {
  int x, y;
  String s((const __FlashStringHelper*) message);
  tft.fillScreen(BLACK);
  tft.setTextSize(2); tft.setTextFont(2);
  uint16_t len = tft.textWidth(message);
  uint16_t pos = 160 - (len / 2);
  tft.setCursor(pos, 120, 2); tft.setTextSize(2);
  tft.print(message);
  putData("display");
  int light = analogRead(36);
  while (!isRead()) {
    spinServo();
  }
  putData("read");
  while (!isClose(light)) {
    Serial.println("Aspettando");
  }
  fex.drawJpgFile(SPIFFS, "/logo2.jpg", 0, 0);
}

void showImage (const char *url, const char *filename) {
  tft.fillScreen(BLACK);
  String s((const __FlashStringHelper*) url);
  String r((const __FlashStringHelper*) filename);
  bool downloadok = getFile(url, filename);
  fex.drawJpgFile(SPIFFS, filename, 0, 0);
  putData("display");
  int light = analogRead(36);
  while (!isRead()) {
    spinServo();
  }
  putData("read");
  while (!isClose(light)) {
    Serial.println("Aspettando");
  }
  fex.drawJpgFile(SPIFFS, "/logo2.jpg", 0, 0);
}

bool getFile(String url, String filename) {

  // If it exists then no need to fetch it
  if (SPIFFS.exists(filename) == true) {
    Serial.println("Found " + filename);
    return 0;
  }

  Serial.println("Downloading "  + filename + " from " + url);

  // Check WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");

    // Configure server and url
    http.begin(url);

    Serial.print("[HTTP] GET...\n");
    // Start connection and send HTTP header
    int httpCode = http.GET();
    if (httpCode > 0) {
      fs::File f = SPIFFS.open(filename, "w+");
      if (!f) {
        Serial.println("file open failed");
        return 0;
      }
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // File found at server
      if (httpCode == HTTP_CODE_OK) {

        // Get length of document (is -1 when Server sends no Content-Length header)
        int total = http.getSize();
        int len = total;

        // Create buffer for read
        uint8_t buff[128] = { 0 };

        // Get tcp stream
        WiFiClient * stream = http.getStreamPtr();

        // Read all data from server
        while (http.connected() && (len > 0 || len == -1)) {
          // Get available data size
          size_t size = stream->available();

          if (size) {
            // Read up to 128 bytes
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

            // Write it to file
            f.write(buff, c);

            // Calculate remaining bytes
            if (len > 0) {
              len -= c;
            }
          }
          yield();
        }
        Serial.println();
        Serial.print("[HTTP] connection closed or file end.\n");
      }
      f.close();
    }
    else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  return 1; // File was fetched from web
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
    const char *type = (const char *)jobj["type"];
    if (type == "image") {
      const char *url = (const char *)jobj["url"];
      const char *filename = (const char *)jobj["name"];
      showImage(url, filename);
    }
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

  wifiManager.autoConnect("LOVEBOX.configure");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  timeClient.begin();
  timeClient.setTimeOffset(3600);
  server.begin();
  readConfig();
}

void loop() {

  //isRead();
  //spinServo();
  readConfig();
  newMessage();
  delay(10000);


}
