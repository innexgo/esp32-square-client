#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#include "constants.h"

MFRC522 setupMfrc522() {
  Serial.println("MFRC522: Begin initialization");

  pinMode(IRQ_pin, INPUT);
  pinMode(SS_pin, OUTPUT);
  digitalWrite(SS_pin, HIGH);
  pinMode(SCK_pin, OUTPUT);
  pinMode(MOSI_pin, OUTPUT);
  pinMode(MISO_pin, INPUT);
  
  SPI.begin(SCK_pin,MISO_pin,MOSI_pin,SS_pin);

  MFRC522 mfrc522(SS_pin, reset_pin);
  mfrc522.PCD_Init();
  
  Serial.println("MFRC522: Initialized successfully");
  
  return mfrc522;
}



