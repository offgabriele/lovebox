/* Note that this code is specific to ESP32-based boards like
 * the Feather HUZZAH32. Some modifications may be required
 * for different boards and support libraries.
 */
#define FIRMWARE_URL \
"https://firebasestorage.googleapis.com/v0/b/your-project.appspot.com/o/AwesomeProject.ino.bin?alt=media&token=836b2515-2000-473f-a5ff-bf9c38b958c9"
#include "Update.h"
#include "WiFi.h"
#include "HTTPClient.h"
void updateFirmware() {
  // Start pulling down the firmware binary.
  http.begin(FIRMWARE_URL);
  int httpCode = http.GET();
  if (httpCode <= 0) {
    Serial.printf("HTTP failed, error: %s\n", 
       http.errorToString(httpCode).c_str());
    return;
  }
  // Check that we have enough space for the new binary.
  int contentLen = http.getSize();
  Serial.printf("Content-Length: %d\n", contentLen);
  bool canBegin = Update.begin(contentLen);
  if (!canBegin) {
    Serial.println("Not enough space to begin OTA");
    return;
  }
  // Write the HTTP stream to the Update library.
  WiFiClient* client = http.getStreamPtr();
  size_t written = Update.writeStream(*client);
  Serial.printf("OTA: %d/%d bytes written.\n", written, contentLen);
  if (written != contentLen) {
    Serial.println("Wrote partial binary. Giving up.");
    return;
  }
  if (!Update.end()) {
    Serial.println("Error from Update.end(): " + 
      String(Update.getError()));
    return;
  }
  if (Update.isFinished()) {
    Serial.println("Update successfully completed. Rebooting."); 
    // This line is specific to the ESP32 platform:
    ESP.restart();
  } else {
    Serial.println("Error from Update.isFinished(): " + 
      String(Update.getError()));
    return;
  }
}