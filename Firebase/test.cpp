void showMessage (const char *message) {
  int x, y;
  String s((const __FlashStringHelper*) message);
  while (!isRead) {
  tft.begin();tft.setRotation(3);tft.fillScreen(BLACK); tft.setTextSize(2);tft.setTextDatum(BC_DATUM);tft.setCursor(10, 110, 2);tft.println(s);
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);
  }
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
}}
