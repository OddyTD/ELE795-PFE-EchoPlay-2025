#pragma once
#include <SD.h>
#include <WiFi.h>
#include <WebSocketsClient.h>

#include "LGFX_ESP32.hpp"
#include "secrets.hpp"

class UnitTest
{
public:
  UnitTest(LGFX& tft);

  void testSD();
  void testWifi(const char* ssid = WIFI_SSID, const char* password = WIFI_PASSWORD);
  void testWebSocket(const char* host = WEBSOCKET_HOST, uint16_t port = WEBSOCKET_PORT);
  void testAffichage();
  void testTactile();

private:
  LGFX& tft;

  // --- Broches SPI utilisées pour la carte SD ---
  static constexpr int PIN_CS = 5;    // Chip Select pour SD (doit être unique)
  static constexpr int PIN_MOSI = 23; // Partagé avec l'écran
  static constexpr int PIN_MISO = 19; // Partagé avec l'écran
  static constexpr int PIN_SCLK = 18; // Partagé avec l'écran
};
