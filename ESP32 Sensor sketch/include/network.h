#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>
#include <WiFiUdp.h>

extern WiFiUDP udp;
extern IPAddress pcIP;
extern const uint16_t udpPort;

extern const char* ssid ;
extern const char* password;



void setupWiFi();
void sendData(const char* packet);

#endif
