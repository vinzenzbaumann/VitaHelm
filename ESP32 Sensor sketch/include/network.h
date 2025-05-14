// network.h
#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>
#include <WiFiUdp.h>

extern WiFiUDP udp;
extern IPAddress pcIP;
extern const int udpPort;

void setupWiFi();
void sendData(const char* packet);

#endif
