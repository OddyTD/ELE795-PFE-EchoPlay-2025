#include "hardware.hpp"


HardwareConfig::HardwareConfig(LGFX& tft) :
  tft(tft)
{}

void HardwareConfig::ConfigEcran()
{
  tft.begin();
  tft.setRotation(1);
  tft.setBrightness(255);
}

bool HardwareConfig::ConfigCarteSD()
{
  // Initialise le bus SPI partagé avec l'écran
  SPI.begin(PIN_SCLK, PIN_MISO, PIN_MOSI);

  if (!SD.begin(PIN_CS))
  {
    Serial.println("[Erreur] Carte SD non détectée");
    return false;
  }
  Serial.println("[OK] Carte SD initialisée");
  return true;
}

void HardwareConfig::ConfigWiFi(const char* ssid, const char* password)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connexion WiFi en cours");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 5000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("\nWiFi connecté.");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());
}