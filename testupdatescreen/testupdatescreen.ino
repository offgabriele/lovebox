#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>
#include <Servo.h>
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

void setup() {
  // put your setup code here, to run once:
  tft.begin(); tft.setRotation(3); tft.fillScreen(BLACK);
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");
  fex.listSPIFFS();
}

void loop() {
  // put your main code here, to run repeatedly:
      fex.drawJpgFile(SPIFFS, "/update.jpg", 0, 0);  
      tft.setTextColor(TFT_BLACK); 
      tft.setTextSize(3); tft.setTextFont(2);
      uint16_t len = tft.textWidth("Updating...");
      uint16_t pos = 160 - (len/2);
      tft.setCursor(pos, 170, 2); tft.setTextSize(3); tft.print("Updating...");
    
    delay(10000);
}
