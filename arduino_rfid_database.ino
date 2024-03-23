#include <StreamUtils.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX

MFRC522 mfrc522(10, 9);

const int RedledPin = 7; // Red LED connected to digital pin 7
const int GreenledPin = 4; // Green LED connected to digital pin 4

bool isRead = false;
bool isNewCard = false;
String tagContent = "";
String currentUID = "";

// Interval before we process the same RFID
int INTERVAL = 2000;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

void setup() {
  Serial.begin(115200);
  mySerial.begin(4800);
  mySerial.setTimeout(5000);

  pinMode(RedledPin, OUTPUT);
  pinMode(GreenledPin, OUTPUT);

  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("Detecting RFID Tags");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      isRead = true;

      tagContent = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        tagContent.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        tagContent.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      tagContent.toUpperCase();
    }
    
    if (isRead) {
      currentMillis = millis();
      if (currentUID != tagContent) {
        currentUID = tagContent;
        isNewCard = true;
      } else {
        if (currentMillis - previousMillis >= INTERVAL) {
          isNewCard = true;
        } else {
          isNewCard = false;
        }
      }

      if (isNewCard) {
        if (tagContent != "") {
          digitalWrite(GreenledPin, LOW);
          previousMillis = currentMillis;
          Serial.print("Sending data to ESP-01: ");
          Serial.println(tagContent);
          mySerial.println(tagContent);
          Serial.println("Waiting for response from ESP-01...");
          
          int iCtr = 0;
          while (!mySerial.available()) {
            iCtr++;
            if (iCtr >= 100)
              break;
            delay(50);
          }
          
          if (mySerial.available()) {
            String response = mySerial.readStringUntil('\n');
            Serial.print("Response from ESP-01: ");
            Serial.println(response);
            
            if (response.startsWith("payment done")) {
              digitalWrite(GreenledPin, HIGH);
              delay(1000);
              digitalWrite(GreenledPin, LOW);
            } else {
              tone(RedledPin, 2000);
              delay(500);
              noTone(RedledPin);
            }
          }
          Serial.println("Finished processing response from ESP-01");
        }
      }
    } else {
      Serial.println("No card details were read!");
    }
    tagContent = "";
    isNewCard = false;
  }
}