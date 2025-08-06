#include "hardware.hpp"
#include "wifi_credentials.hpp"

void HardwareInit::initEcran(LGFX &tft)
{
  tft.begin();
  tft.setRotation(1);
  tft.setBrightness(255);
  // tft.fillScreen(TFT_DARKGREEN);
}

bool HardwareInit::initCarteSD()
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

void HardwareInit::initWiFi()
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