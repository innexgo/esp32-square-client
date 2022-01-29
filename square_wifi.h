#ifndef SQUARE_WIFI_H
#define SQUARE_WIFI_H

#include <WiFi.h>

const char* ssid  = "alpha";
const char* password = "braincontrol";

void connectWiFi() {
  // Connect to wifi
  WiFi.begin(ssid, password);
  Serial.println("WiFi: Begin Initialization");
  // Wait some time to connect to wifi
  while(true) {
    Serial.println("WiFi: Connecting...");
    delay(100);
    if(WiFi.status() == WL_CONNECTED) {
      break;
    }
  }
  Serial.println("WiFi: Connected!");
}

#endif
