#include "hardware.hpp"
#include "secrets.hpp"

void HardwareConfig::ConfigEcran(LGFX &tft)
{
  tft.begin();
  tft.setRotation(1);
  tft.setBrightness(255);
  // tft.fillScreen(TFT_DARKGREEN);
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

void HardwareConfig::ConfigWiFi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connexion WiFi en cours");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connecté.");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());
}