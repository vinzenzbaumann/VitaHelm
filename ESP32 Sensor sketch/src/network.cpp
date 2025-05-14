// network.cpp
#include "network.h"



//Wlan Config
WiFiUDP udp;
IPAddress pcIP(172, 16, 19, 11);  // Ziel-IP des PCs (hier jetzt Vinzenz Mac)
const int udpPort = 4210;

void setupWiFi() {
    WiFi.begin("TI Roboter", "ITRobot!"); 
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Serial.println("Verbunden mit WLAN");
}

void sendData(const char* packet) {
    udp.beginPacket(pcIP, udpPort);
    udp.write((const uint8_t*)packet, strlen(packet));
    udp.endPacket();
}
