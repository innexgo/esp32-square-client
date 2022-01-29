#include <Arduino.h>
#include <MFRC522.h>
#include <HTTPClient.h>

#include "constants.h"

#include "square_mfrc522.h"
#include "square_wifi.h"

void beep(int totalwidth, int cycwidth) {
  int cycles = totalwidth/cycwidth;
  for(int i = 0; i < cycles; i++) {
    digitalWrite(buzzer_pin, HIGH);
    delayMicroseconds(cycwidth/2);
    digitalWrite(buzzer_pin, LOW);
    delayMicroseconds(cycwidth/2);
  } 
}

void beepDown() {
  beep(100000, 1000);
  beep(100000, 2000);
}

void beepUp() {
  beep(100000, 2000);
  beep(100000, 1000);
}

void beepError() {
  beep(100000, 1000);
  beep(100000, 2000);
  beep(100000, 1000);
}

void sendEncounter(char* hostname, char* apiKey, long studentId, long locationId) {
  char url_buffer[1024];
  sprintf(
    url_buffer, 
    (
        "%s"
        "/api/misc/attends/"
        "?manual=false"
        "&apiKey=%s"
        "&locationId=%d"
        "&studentId=%d"
    ),
    hostname, apiKey, studentId, locationId
  );

  HTTPClient http;

  http.begin(url_buffer);

  // Send HTTP GET request
  int httpResponseCode = http.GET();

  Serial.println(httpResponseCode);
  Serial.println(http.getString());
}


MFRC522 mfrc522;

void setup() {
  // set up pins
  pinMode(reset_pin, OUTPUT);
  digitalWrite(reset_pin, HIGH);
  pinMode(status_pin_1, OUTPUT); 
  pinMode(status_pin_2, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);

  // begin serial
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Initialized Serial");

  // setup mfrc522
  mfrc522 = setupMfrc522();

  // setup wifi
  connectWiFi();
}

void loop() {  
  
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  byte PSWBuff[] = {0xFF, 0xFF, 0xFF, 0xFF}; // 32 bit password default FFFFFFFF.
  byte pACK[] = {0, 0}; // 16 bit password ACK returned by the NFCtag.

  Serial.print("Auth: ");
  Serial.println(mfrc522.PCD_NTAG216_AUTH(&PSWBuff[0], pACK)); // Request authentification if return STATUS_OK we are good.

  //Print PassWordACK
  Serial.print(pACK[0], HEX);
  Serial.println(pACK[1], HEX);

  byte WBuff[] = {0x00, 0x00, 0x00, 0x04};
  byte RBuff[18];

  //Serial.print("CHG BLK: ");
  //Serial.println(mfrc522.MIFARE_Ultralight_Write(0xE3, WBuff, 4));  // How to write to a page.

  mfrc522.PICC_DumpMifareUltralightToSerial(); // This is a modifier dump just change the for circle to < 232 instead of < 16 in order to see all the pages on NTAG216.
  
  beepUp();
  

  delay(300);
}
