#pragma once
#include <cstdint>  // Pour uint16_t, int32_t, etc.

#include "LGFX_ESP32.hpp"
#include "websocket.hpp"
#include "menu.hpp"

// Classe EtatReseau : gère l'affichage de l'état de connexion 
class EtatReseau {
public:
  EtatReseau();

  void mettreAJour(LGFX& tft, WebSocket& wsClient, Menu& menu);

private:
  unsigned long derniereActualisation = 0;
  static constexpr unsigned long intervalleActualisation = 500;
};