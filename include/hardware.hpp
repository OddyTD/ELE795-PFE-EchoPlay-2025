#pragma once
#include <SD.h>
#include <WiFi.h>

#include "LGFX_ESP32.hpp"
#include "gestion_reseau.hpp"
#include "secrets.hpp"

/**********************************************************
 *  Classe : HardwareInit                                   *
 *  Description :                                           *
 *    Classe utilitaire pour initialiser le matériel       *
 *    (écran, carte SD, WiFi)                              *
 **********************************************************/
class HardwareConfig
{
public:
  HardwareConfig(LGFX& tft);

  // Initialise l'écran tactile avec les paramètres par défaut
  void ConfigEcran();

  // Initialise la carte SD pour le stockage des ressources
  bool ConfigCarteSD();

  // Établit la connexion WiFi avec les identifiants configurés
  void ConfigWiFi(const char* ssid = WIFI_SSID, const char* password = WIFI_PASSWORD);

private:
  LGFX& tft;

  // --- Broches SPI utilisées pour la carte SD ---
  static constexpr int PIN_CS = 5;    // Chip Select pour SD (doit être unique)
  static constexpr int PIN_MOSI = 23; // Partagé avec l'écran
  static constexpr int PIN_MISO = 19; // Partagé avec l'écran
  static constexpr int PIN_SCLK = 18; // Partagé avec l'écran
};