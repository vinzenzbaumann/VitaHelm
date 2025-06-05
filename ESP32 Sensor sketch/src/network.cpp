#include "network.h"

// Globale Netzwerkobjekte
WiFiUDP udp;
IPAddress pcIP(172, 16, 19, 11);  // Ziel-IP (z.B. PC)
const uint16_t udpPort = 4210;     // Ziel-Port

// WLAN-Zugangsdaten
const char* ssid = "TI Roboter";
const char* password = "ITRobot!";

// WLAN verbinden
void setupWiFi() {
    WiFi.begin(ssid, password); 
    Serial.print("Verbinde mit WLAN");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nVerbunden mit WLAN");
}

// UDP-Daten senden
void sendData(const char* packet) {
    udp.beginPacket(pcIP, udpPort);
    udp.write((const uint8_t*)packet, strlen(packet));
    udp.endPacket();
}
