#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Chetna-5G";
const char* password = "chetnadeepaksuthar";

void setup() {
  Serial.begin(4800);
  delay(10);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void loop() {
  if (Serial.available()) {
    String rfidCode = Serial.readStringUntil('\n');
    Serial.print("RFID Code received: ");
    Serial.println(rfidCode);

    checkPaymentStatus(rfidCode);
  }
}

void checkPaymentStatus(String rfidCode) {
  HTTPClient http;

  String url = "https://rfid-backend-s195.onrender.com/api/v1/rfid/paymentstatus?rfidID=" + rfidCode;
  Serial.print("Sending GET request to URL: ");
  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.print("Response payload: ");
    Serial.println(payload);

    // Send the response back to Arduino
    Serial.println(payload);
  } else {
    Serial.print("Error in HTTP request: ");
    Serial.println(httpCode);
  }

  http.end();
}