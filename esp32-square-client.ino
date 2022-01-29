#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>
#include <MFRC522.h>
#include <WiFi.h>

#define reset_pin 16
#define status_pin_1 18
#define status_pin_2 22
#define buzzer_pin 17
#define IRQ_pin 13
#define SS_pin 26
#define SCK_pin 25
#define MOSI_pin 33
#define MISO_pin 32

MFRC522 mfrc522(SS_pin, reset_pin);

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.print("Hello There");
  pinMode(reset_pin, OUTPUT);
  digitalWrite(reset_pin, HIGH);
  pinMode(status_pin_1, OUTPUT); 
  pinMode(status_pin_2, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);
  pinMode(IRQ_pin, INPUT);
  
  pinMode(SS_pin, OUTPUT);
  digitalWrite(SS_pin, HIGH);
  pinMode(SCK_pin, OUTPUT);
  pinMode(MOSI_pin, OUTPUT);
  pinMode(MISO_pin, INPUT);
  
  SPI.begin(SCK_pin,MISO_pin,MOSI_pin,SS_pin);
  mfrc522.PCD_Init();
}

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
