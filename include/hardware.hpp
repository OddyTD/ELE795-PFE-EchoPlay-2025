#pragma once
#include "LGFX_ESP32.hpp"
#include <SD.h>
#include <WiFi.h>

/**********************************************************
 *  Classe : HardwareInit                                   *
 *  Description :                                           *
 *    Classe utilitaire pour initialiser le matériel       *
 *    (écran, carte SD, WiFi)                              *
 **********************************************************/
class HardwareInit
{
private:
  // --- Broches SPI utilisées pour la carte SD ---
  static constexpr int PIN_CS = 5;    // Chip Select pour SD (doit être unique)
  static constexpr int PIN_MOSI = 23; // Partagé avec l'écran
  static constexpr int PIN_MISO = 19; // Partagé avec l'écran
  static constexpr int PIN_SCLK = 18; // Partagé avec l'écran

public:
  // Initialise l'écran tactile avec les paramètres par défaut
  static void initEcran(LGFX &tft);

  // Initialise la carte SD pour le stockage des ressources
  static bool initCarteSD();

  // Établit la connexion WiFi avec les identifiants configurés
  static void initWiFi();
};