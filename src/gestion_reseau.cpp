#include <WiFi.h>
#include "gestion_reseau.hpp"

EtatReseau::EtatReseau() {}

void EtatReseau::mettreAJour(LGFX& tft, WebSocket& wsClient, Menu& menu) {
  if (menu.obtenirEtat() != EtatPartie::AttenteConnexion) return;

  unsigned long maintenant = millis();
  if (maintenant - derniereActualisation > intervalleActualisation) {
    bool wifiOK = (WiFi.status() == WL_CONNECTED);
    bool wsOK = wsClient.estPret();

    menu.EtatConnexion(wifiOK, wsOK);
    derniereActualisation = maintenant;
  }
}
