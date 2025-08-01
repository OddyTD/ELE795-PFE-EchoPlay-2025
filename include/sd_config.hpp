#pragma once
#include <SPI.h>
#include <SD.h>

/**********************************************************
*  Classe : SDCardConfig                                   *
*  Description :                                           *
*    Classe utilitaire pour configurer l’accès à la carte  *
*    SD via SPI avec bus partagé avec l’écran TFT.         *
*    Broches compatibles avec le module ESP32-3248S035C.   *
**********************************************************/
class SDCardConfig {
public:
  // --- Broches SPI utilisées pour la carte SD ---
  static constexpr int PIN_CS   = 5;   // Chip Select pour SD (doit être unique)
  static constexpr int PIN_MOSI = 23;  // Partagé avec l’écran
  static constexpr int PIN_MISO = 19;  // Partagé avec l’écran
  static constexpr int PIN_SCLK = 18;  // Partagé avec l’écran

  /**********************************************************
  *  Méthode : begin                                        *
  *  Description :                                          *
  *    Initialise le bus SPI et la carte SD.                *
  *    Retourne true si succès, false sinon.                *
  **********************************************************/
  static bool begin() {
    SPI.begin(PIN_SCLK, PIN_MISO, PIN_MOSI);  // SPI partagé avec écran
    return SD.begin(PIN_CS);                  // Initialisation SD
  }
};
