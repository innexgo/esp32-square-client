#include <Arduino.h>
#include <stdint.h>

#include "constants.h"

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

uint32_t ByteArrayLE_to_uint32(const uint8_t* byteArray) {
  /* casts -before- shifting are necessary to prevent integer promotion 
     and to make the code portable no matter integer size: */

  uint32_t x = (uint32_t)byteArray[0] <<  0 | 
       (uint32_t)byteArray[1] <<  8 | 
       (uint32_t)byteArray[2] << 16 | 
       (uint32_t)byteArray[3] << 24;

  printf("%02X:%02X:%02X:%02X\n", byteArray[0], byteArray[1], byteArray[2], byteArray[3]);
  return x;
}

uint8_t ascii_to_uint8(const byte data) {
  if (data <= 0x39) { // in the 0-9 range
    return data - 0x30;
  } else { // in the A-F range
    return data - 0x37;
  }
}

void setup() {
  // set up pins
  pinMode(status_pin_1, OUTPUT); 
  pinMode(status_pin_2, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);

  
  // begin serial
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Initialized Serial");

  Serial1.begin(9600, SERIAL_8N1, serial_rxd1, serial_txd1);
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
  byte inputBuffer[inputBuffer_size];
  byte cardDataBuffer[6];
  uint8_t facilityCode = 0;
  uint32_t cardNumber = 0;

  if (Serial1.available() > 0) {
    memset(inputBuffer, 0, inputBuffer_size*sizeof(*inputBuffer));
    Serial1.readBytesUntil('\03', inputBuffer, inputBuffer_size);

    if (inputBuffer[0] != '\02') {
      return; // errored
    }
    if (inputBuffer[1] == '2') { // HID Card
      for (int i = 3; i < 14; i += 2) {
        cardDataBuffer[(i-3)/2] = (ascii_to_uint8(inputBuffer[i]) << 4) + ascii_to_uint8(inputBuffer[i+1]);
      }
      if (cardDataBuffer[0] == 0x00 && cardDataBuffer[1] == 0x20 && (cardDataBuffer[2] & 0xfc) == 0x04) {
        // H10301 format
        // TODO: check parity bits
        facilityCode = ((cardDataBuffer[2] & 0x01) << 7) + ((cardDataBuffer[3] & 0xfe) >> 1);
        cardNumber = ((cardDataBuffer[3] & 0x01) << 15) + (cardDataBuffer[4] << 7) + ((cardDataBuffer[5] & 0xfe) >> 1);
      } else {
        // unprogrammed HID card format
        return;
      }
    } else if (inputBuffer[1] == '1') { // EM Card
      // TODO: debate on handling
      return; // for now
      // for (int i = 3; i < 12; i += 2) {
      //   cardDataBuffer[(i-3)/2] = (ascii_to_uint8(inputBuffer[i]) << 4) + ascii_to_uint8(inputBuffer[i+1]);
      // }
    } else {
      // handle error
      return;
    }

    bool signin;
    bool success = sendEncounter(cardNumber, &signin);
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
  }

  // Delay unnecessary; TBD
  // delay(200);
}
