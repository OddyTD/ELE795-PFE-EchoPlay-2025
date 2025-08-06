#include <vector>
#include <WebSocketsClient.h>
#include "LGFX_ESP32.hpp"
#include "gestion_cartes.hpp"
#include "menu.hpp"
#include "audio.hpp"

#include "websocket.hpp"
#include "hardware.hpp"

const char *WEBSOCKET_HOST = "192.168.1.58"; // PC
const uint16_t WEBSOCKET_PORT = 8765;

LGFX tft;
MainJoueur mainJoueur;
Menu menuBas;
WebSocket wsClient;

HardwareInit Hardware;

unsigned long dernierRefresh = 0;
const unsigned long intervalleRefresh = 500;
String resultatPartie = "";

void setup()
{
  Serial.begin(115200);

  // Initialisation de l'écran, de la carte SD et du Wi-Fi
  Hardware.initEcran(tft);
  Hardware.initCarteSD();
  Hardware.initWiFi();

  // pinMode(PIN_AUDIO, OUTPUT);
  // digitalWrite(PIN_AUDIO, LOW);

  // Démarrer la connexion WebSocket et configurer les callbacks
  wsClient.demarrer(WEBSOCKET_HOST, WEBSOCKET_PORT);
  wsClient.callbackLogiqueJeu(mainJoueur, menuBas, tft);

  // Change l'état à "Attente de connexion" et affiche l'écran de connexion
  menuBas.definirEtat(EtatPartie::AttenteConnexion);
  menuBas.afficherEcranConnexion(tft, wsClient.estPret());
}

void loop()
{
  wsClient.actualiser();

  unsigned long maintenant = millis();
  if (!wsClient.estPret() && maintenant - dernierRefresh > intervalleRefresh)
  {
    bool wifiOK = (WiFi.status() == WL_CONNECTED);
    menuBas.rafraichirEtatConnexion(tft, wifiOK);
    dernierRefresh = maintenant;
  }

  int tx, ty;
  if (tft.getTouch(&tx, &ty))
  {
    ActionMenu action = menuBas.gererAction(tft, tx, ty);

    // Ne permet que Rejouer si la partie est terminée
    if (menuBas.obtenirEtat() == EtatPartie::Terminee && action != ActionMenu::Rejouer)
    {
      return;
    }

    if (action == ActionMenu::Connexion)
    {
      if (wsClient.estPret())
      {
        Serial.println("[ESP32] ✅ Joueur prêt");

        wsClient.envoyerAction(ActionWebSocket::Pret);
        tft.fillScreen(TFT_DARKGREEN);
        menuBas.afficherActions(tft);
        menuBas.definirEtat(EtatPartie::EnCours);
      }
      else
      {
        Serial.println("[ESP32] ⚠️ Serveur non disponible");
        menuBas.afficherMessageTemporaire(tft, "Serveur hors-ligne");
        delay(5000);
        menuBas.afficherEcranConnexion(tft, false);
      }
    }

    else if (action == ActionMenu::Draw && wsClient.estPret())
    {
      wsClient.envoyerAction(ActionWebSocket::TirerCarte);
    }

    else if (action == ActionMenu::Stand && wsClient.estPret())
    {
      // TODO: Implémenter action "Stand"
    }

    else if (action == ActionMenu::Rejouer)
    {
      Serial.println("[ESP32] 🔁 Nouvelle partie demandée");

      mainJoueur.reinitialiser(tft);
      menuBas.setMise(1);
      menuBas.afficherEcranConnexion(tft, wsClient.estPret());
      menuBas.definirEtat(EtatPartie::AttenteConnexion);

      wsClient.envoyerAction(ActionWebSocket::Rejouer);
    }

    else if (action == ActionMenu::AugmenterMise || action == ActionMenu::DiminuerMise)
    {
      menuBas.afficherMise(tft);
    }

    delay(200); // Anti-rebond
  }
}
