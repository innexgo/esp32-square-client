#include <Arduino.h>
#include <MFRC522.h>
#include <stdint.h>

#include "constants.h"
#include "endian.h"

#include "square_mfrc522.h"
#include "square_wifi.h"

void beep(int totalwidth, int cycwidth) {
  int cycles = totalwidth/cycwidth;
  for(int i = 0; i < cycles; i++) {

    //TEMP disable beeps.
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
  beep(100000, 2000);
  delayMicroseconds(100000);
  beep(100000, 2000);
  delayMicroseconds(100000);
  beep(100000, 2000);
}

void longToByteArray(long val, byte* buf)
{
    for (int i = 0; i < 16; i++)
    {
        buf[i] = ((val >> (8 * i)) & 0XFF);
    }
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
  Serial.setTimeout(600000);
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
  byte RBuff[18];
  byte WBuff[16];
  byte bufferSize = sizeof(RBuff);
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  // if ( ! mfrc522.PICC_IsNewCardPresent()) {
  //   return;
  // }

  // Select one of the cards
  // if ( ! mfrc522.PICC_ReadCardSerial()) {
  //   return;
  // }
  

  Serial.print("Scan Barcode: ");
  while (Serial.available() == 0) {
  }

  String inputString = Serial.readStringUntil('\n');
  Serial.println("Recieved: " + inputString);
  long scannedId = inputString.toInt();
  if (scannedId == 0) {
    Serial.print("Invalid Input, scan again.\n");
    beepError();
    return;
  }

  uint32_t scanned_le = htole32(scannedId);
  uint32_t* scanned_le_addr = &scanned_le;
  // clear write buffer
  memset(WBuff, 0, 16*sizeof(byte));
  memcpy(WBuff, (byte*)scanned_le_addr, 16);
  Serial.printf("%02X, %02X, %02X, %02X\n", WBuff[0], WBuff[1], WBuff[2], WBuff[3]);

  byte PSWBuff[] = {0xFF, 0xFF, 0xFF, 0xFF}; // 32 bit password default FFFFFFFF.
  byte pACK[] = {0, 0}; // 16 bit password ACK returned by the NFCtag.
  
  Serial.print("Scan Card...\n");
  // wait for card
  while(true) {
    if(!mfrc522.PICC_IsNewCardPresent()) {
      continue;
    }
    if(!mfrc522.PICC_ReadCardSerial()) {
      continue;
    }
    break;
  }

  Serial.print("Auth: ");
  Serial.println(mfrc522.PCD_NTAG216_AUTH(&PSWBuff[0], pACK)); // Request authentification if return STATUS_OK we are good.

  //Print PassWordACK
  Serial.print("PassWordACK: ");
  Serial.print(pACK[0], HEX);
  Serial.println(pACK[1], HEX);

  // Read from sector 10
  memset(RBuff, 0, 18*sizeof(byte)); //clear buffer
  mfrc522.MIFARE_Read(2*4, RBuff, &bufferSize);
  printf("Original: %02X:%02X:%02X:%02X\n", RBuff[8], RBuff[9], RBuff[10], RBuff[11]);

  MFRC522::StatusCode writeStatus = mfrc522.MIFARE_Write(10, WBuff, 16);

  memset(RBuff, 0, 18*sizeof(byte)); //clear buffer
  MFRC522::StatusCode readStatus = mfrc522.MIFARE_Read(2*4, RBuff, &bufferSize);
  printf("Written: %02X:%02X:%02X:%02X\n", RBuff[8], RBuff[9], RBuff[10], RBuff[11]);

  uint32_t studentId = le32toh(*(uint32_t*)(RBuff+8));
  printf("Parsed Value: %i\n", studentId);
  printf("Write: %s Read: %s\n", MFRC522::GetStatusCodeName(writeStatus), MFRC522::GetStatusCodeName(readStatus));
  printf("Read Buffer: %i\n", mfrc522.PCD_ReadRegister(MFRC522::FIFOLevelReg));
  printf("Read Buffer Size: %i\n", *&bufferSize);
  
  if (scanned_le == studentId) {
    beepUp();
  } else {
    beepError();
  }
  //mfrc522.PICC_DumpMifareUltralightToSerial(); // This is a modifier dump just change the for circle to < 232 instead of < 16 in order to see all the pages on NTAG216.

  // bool signedin;
  // bool success = sendEncounter(studentId, &signedin); 
  // if(success) {
  //   if(signedin) {
  //     beepUp();
  //   } else {
  //     beepDown();
  //   }
  // } else {
  //   beepError();
  // }
  
  delay(200);
}
