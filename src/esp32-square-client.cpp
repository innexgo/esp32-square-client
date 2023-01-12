#include <Arduino.h>
#include <MFRC522.h>
#include <stdint.h>

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
  digitalWrite(status_pin_2, HIGH);
  beep(100000, 1000);
  digitalWrite(status_pin_2, LOW);
  delay(100);
  digitalWrite(status_pin_2, HIGH);
  beep(100000, 2000);
  beep(100000, 2000);
  digitalWrite(status_pin_2, LOW);
}

void beepUp() {
  digitalWrite(status_pin_2, HIGH);
  beep(100000, 2000);
  beep(100000, 1000);
  digitalWrite(status_pin_2, LOW);
}

void beepError() {
  digitalWrite(status_pin_1, HIGH);
  beep(100000, 2000);
  delayMicroseconds(100000);
  beep(100000, 2000);
  delayMicroseconds(100000);
  beep(100000, 2000);
  digitalWrite(status_pin_1, LOW);
}

uint32_t ByteArrayLE_to_uint32 (const uint8_t* byteArray) {
  /* casts -before- shifting are necessary to prevent integer promotion 
     and to make the code portable no matter integer size: */

  uint32_t x = (uint32_t)byteArray[0] <<  0 | 
       (uint32_t)byteArray[1] <<  8 | 
       (uint32_t)byteArray[2] << 16 | 
       (uint32_t)byteArray[3] << 24;

  printf("%02X:%02X:%02X:%02X\n", byteArray[0], byteArray[1], byteArray[2], byteArray[3]);
  return x;
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

  // while(true) {
  //   digitalWrite(status_pin_1, HIGH);
  //   digitalWrite(status_pin_2, HIGH);
  //   delay(100);
  //   digitalWrite(status_pin_1, LOW);
  //   digitalWrite(status_pin_2, LOW);
  //   delay(100);
  // }

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
  Serial.print("PassWordACK: ");
  Serial.print(pACK[0], HEX);
  Serial.println(pACK[1], HEX);



  // Read from sector 10 
  byte RBuff[18];
  memset(RBuff, 0, 18*sizeof(byte)); 
  byte bufferSize = sizeof(RBuff);

  // for(int a = 0; a < 4; a++) {
  mfrc522.MIFARE_Read(2*4, RBuff, &bufferSize);

  // printf("%02X:%02X:%02X:%02X\n", RBuff[0], RBuff[1], RBuff[2], RBuff[3]);
  // printf("%02X:%02X:%02X:%02X\n", RBuff[4], RBuff[5], RBuff[6], RBuff[7]);
  // printf("%02X:%02X:%02X:%02X\n", RBuff[8], RBuff[9], RBuff[10], RBuff[11]);
  // printf("%02X:%02X:%02X:%02X\n", RBuff[12], RBuff[13], RBuff[14], RBuff[15]);

    //Serial.print(RBuff[i]);
  
  // }

  byte SIDBuff[4];
  memset(SIDBuff, 0, 4*sizeof(byte));
  memcpy(SIDBuff, &RBuff[8], sizeof(SIDBuff));
  uint32_t studentId = ByteArrayLE_to_uint32(SIDBuff);//le32toh(*(uint32_t*)RBuff);

  //mfrc522.PICC_DumpMifareUltralightToSerial(); // This is a modifier dump just change the for circle to < 232 instead of < 16 in order to see all the pages on NTAG216.

  bool signin;
  bool success = sendEncounter(studentId, &signin);
  if(success) {
    if(signin) {
      beepUp();
    } else {
      beepDown();
    }
    // delay so user doesn't accidentally rescan
    delay(1000);
  } else {
    beepError();
  }
  // Delay unnecessary; TBD
  // delay(200);
}
