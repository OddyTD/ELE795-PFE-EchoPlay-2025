#include <WiFi.h>
#include "gestion_reseau.hpp"

EtatReseau::EtatReseau() {}

void EtatReseau::mettreAJour(LGFX& tft, WebSocket& wsClient, Menu& menu) {
  unsigned long maintenant = millis();

  if (!wsClient.estPret() && maintenant - derniereActualisation > intervalleActualisation) {
    bool wifiOK = (WiFi.status() == WL_CONNECTED);
    menu.rafraichirEtatConnexion(tft, wifiOK);
    derniereActualisation = maintenant;
  }
}
