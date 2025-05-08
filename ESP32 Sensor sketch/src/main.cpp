#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

// WLAN-Konfiguration
WiFiUDP udp;
IPAddress pcIP(172, 16, 19, 11);  // Ziel-IP deines PCs
const int udpPort = 4210;

// ADXL335 Pins
#define ADXL_X_PIN 32
#define ADXL_Y_PIN 33
#define ADXL_Z_PIN 34

// Mikrofon-Pegeltrigger (D0 vom KY-038 → z. B. GPIO 25)
#define MICROPHONE_DIGITAL_PIN 25

// MX30102 I2C-Adresse
#define MX30102_ADDR 0xAE  // Adresse aus Scan oder Dokumentation

// Timer
hw_timer_t *timer = NULL;
volatile bool sendData = false;

void IRAM_ATTR onTimer() {
  sendData = true;
}

// I2C-Scanner
void scanI2CDevices() {
  Serial.println("I2C-Scan startet...");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C-Gerät gefunden bei Adresse 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
    }
  }
  Serial.println("I2C-Scan abgeschlossen.\n");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // WLAN verbinden
  WiFi.begin("TI Roboter", "ITRobot!");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("Verbunden mit WLAN");

  // ADXL335 vorbereiten
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(MICROPHONE_DIGITAL_PIN, INPUT);

  // I2C starten (ESP32: SDA = 21, SCL = 22)
  Wire.begin(21, 22);
  scanI2CDevices();  // Scan beim Start durchführen

  // Timer auf 100 Hz
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 10000, true);
  timerAlarmEnable(timer);

  Serial.println("Programm läuft...");
}

void loop() {
  static unsigned long lastSendTime = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastSendTime >= 1000) {
    udp.beginPacket(pcIP, udpPort);
    udp.write((const uint8_t*)"1", 1);
    udp.endPacket();
    Serial.println("Sende: 1");
    lastSendTime = currentMillis;
  }

  if (sendData) {
    uint16_t x = analogRead(ADXL_X_PIN);
    uint16_t y = analogRead(ADXL_Y_PIN);
    uint16_t z = analogRead(ADXL_Z_PIN);
    int micTrigger = digitalRead(MICROPHONE_DIGITAL_PIN);

    // MX30102 auslesen
    uint8_t data[6] = {0};
    Wire.beginTransmission(MX30102_ADDR);
    Wire.write(0x00);  // Standardbefehl (prüfen im Datenblatt!)
    Wire.endTransmission();
    delay(100);
    Wire.requestFrom(MX30102_ADDR, 6);
    int pulse = 0;
    int oxygenSaturation = 0;
    if (Wire.available() >= 6) {
      for (int i = 0; i < 6; i++) {
        data[i] = Wire.read();
      }
      pulse = data[0];
      oxygenSaturation = data[1];
    }

    char packet[128];
    snprintf(packet, sizeof(packet), "X:%u,Y:%u,Z:%u,MicTrigger:%d,Pulse:%d,Oxygen:%d\n", x, y, z, micTrigger, pulse, oxygenSaturation);

    udp.beginPacket(pcIP, udpPort);
    udp.write((uint8_t*)packet, strlen(packet));
    udp.endPacket();

    Serial.printf("Sende Daten: X=%u, Y=%u, Z=%u, Mikrofon-Trigger: %d, Puls: %d, Sauerstoff: %d\n",
                  x, y, z, micTrigger, pulse, oxygenSaturation);

    sendData = false;
  }
}
