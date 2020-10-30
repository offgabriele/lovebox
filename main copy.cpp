#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

const char FIRMWARE_VERSION[] = ("0-0-1");
const char* fingerprint = "03 D6 42 23 03 D1 0C 06 73 F7 E2 BD 29 47 13 C3 22 71 37 1B";
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFnDCCBISgAwIBAgIRAPDL8o/fIlacCAAAAAAv7iIwDQYJKoZIhvcNAQELBQAw\n" \
"QjELMAkGA1UEBhMCVVMxHjAcBgNVBAoTFUdvb2dsZSBUcnVzdCBTZXJ2aWNlczET\n" \
"MBEGA1UEAxMKR1RTIENBIDFPMTAeFw0yMDAyMjAxOTM2MDZaFw0yMTAyMTgxOTM2\n" \
"MDZaMGgxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpDYWxpZm9ybmlhMRYwFAYDVQQH\n" \
"Ew1Nb3VudGFpbiBWaWV3MRMwEQYDVQQKEwpHb29nbGUgTExDMRcwFQYDVQQDEw5m\n" \
"aXJlYmFzZWlvLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAN9R\n" \
"hBIBdkeJFsy84i71J+TSRC0699jyn/JElfSpV/TYsmbrgzYdtZSZoCE+F85kWmHr\n" \
"fy/wXL1EWgO54XSnvSjwUZ5ZzphbZuMOASJQo6qeVzXQrBH2LVinE8RJ+jyJZWBu\n" \
"+egQlXE3SvoRD08b+n2M2+wvStTuKoalLxFEheNrbpSpvlrG/TCC7rsrPGm9Kzrn\n" \
"gxXBPLarq2LcE/ibUUHfIcZwuk298HdgFjRWD8+1zykkRg0hBuY3C02ujsiNLkN+\n" \
"gSBQLHD2VRIsewFiG7MvZsEdYYxWEwywkDoKAgBMIhszFDXf3UHDGmp96j7scz5P\n" \
"+1pGQJgQNU2Kc6s7Y+kCAwEAAaOCAmUwggJhMA4GA1UdDwEB/wQEAwIFoDATBgNV\n" \
"HSUEDDAKBggrBgEFBQcDATAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBTaIo3cokKr\n" \
"ewVKbvbTYD7SWCwTQjAfBgNVHSMEGDAWgBSY0fhuEOvPm+xgnxiQG6DrfQn9KzBk\n" \
"BggrBgEFBQcBAQRYMFYwJwYIKwYBBQUHMAGGG2h0dHA6Ly9vY3NwLnBraS5nb29n\n" \
"L2d0czFvMTArBggrBgEFBQcwAoYfaHR0cDovL3BraS5nb29nL2dzcjIvR1RTMU8x\n" \
"LmNydDArBgNVHREEJDAigg5maXJlYmFzZWlvLmNvbYIQKi5maXJlYmFzZWlvLmNv\n" \
"bTAhBgNVHSAEGjAYMAgGBmeBDAECAjAMBgorBgEEAdZ5AgUDMC8GA1UdHwQoMCYw\n" \
"JKAioCCGHmh0dHA6Ly9jcmwucGtpLmdvb2cvR1RTMU8xLmNybDCCAQMGCisGAQQB\n" \
"1nkCBAIEgfQEgfEA7wB1APZclC/RdzAiFFQYCDCUVo7jTRMZM7/fDC8gC8xO8WTj\n" \
"AAABcGRQkc4AAAQDAEYwRAIgAqAKqje0yTE/6UGDNWPv0+5F4tnO5A7opAGi/JyR\n" \
"UfECIGArAusuTUnG4NKHM7C67015o766qReSFmomT/wAUFxRAHYARJRlLrDuzq/E\n" \
"QAfYqP4owNrmgr7YyzG1P9MzlrW2gagAAAFwZFCSJwAABAMARzBFAiBNBYTgBcqm\n" \
"FFGdxj5dhNamhICADLcc3Ubic2FAweOQ0QIhAM1MtmXumV+xvh3WRYMM9DOrXIR7\n" \
"6qzQYcdI8s5EnNdkMA0GCSqGSIb3DQEBCwUAA4IBAQCTa6t2LdK7gRuRKEl4vY/r\n" \
"NhPmbPLkkiZoCeYFjsvXYmo/HpeMR1etRi4WpXgno0lG8a52YIPBu3VlGJvHterX\n" \
"+SRNkPy5V+3Bc2qI6kGr/AzTljmOVsqjo0Xp1nCtHOUVFQZ0HrnGDVEKrjFvCT9Q\n" \
"aVkHQ/9ua+0Boxz9TiGdBeu4dI2nQ9uyxQJ56wqNGrkIOQ76bUsFkqYVk/6HgcmA\n" \
"rFYBGtQHhfFw/HZXfe9+eD3F6f4P1S7TuCcGtIVYsvU/izHf9qpbC6sDnAqy+8KQ\n" \
"r/sioXuPbGT4x7SFahcrkczLB3V9/vV2yNKejsE32edzyjx6BFnqWciTi8t6eFHA\n" \
"-----END CERTIFICATE-----\n";

// Allocate a 1024-byte buffer for the JSON document.
StaticJsonDocument<1024> jsonDoc;
HTTPClient http;
void updateFirmware(String firmwareUrl) {
  http.begin(firmwareUrl, fingerprint);
  int httpCode = http.GET();
  if (httpCode <= 0) {
    Serial.printf("HTTP failed, error: %s\n", 
       http.errorToString(httpCode).c_str());
    return;
  }
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
  Serial.println(url);
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
 http.setTimeout(10000);
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
 Serial.println(nextVersion);
 if (strcmp(nextVersion, FIRMWARE_VERSION) &&          
     strcmp(nextVersion, "none") &&        
     strcmp(nextVersion, "") &&        
     strcmp(nextVersion, "current")) {      
   startFirmwareUpdate(nextVersion);
 }
 else {
  Serial.println("Latest version installed");
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
 

  Serial.println("ciaogab");

  delay(10000);
 
}