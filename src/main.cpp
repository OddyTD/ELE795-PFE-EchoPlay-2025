#include <vector>
#include <WebSocketsClient.h>
#include "LGFX_ESP32.hpp"
#include "gestion_cartes.hpp"
#include "menu.hpp"
#include "audio.hpp"

#include "hardware.hpp"
#include "websocket.hpp"

LGFX tft;
MainJoueur mainJoueur;
Menu menuBas(tft);
WebSocket wsClient;
HardwareConfig Hardware;
EtatReseau IndicateurConnexion;

unsigned long dernierRefresh = 0;
const unsigned long intervalleRefresh = 500;
String resultatPartie = "";

void setup()
{
  Serial.begin(115200);

  // Initialisation de l'écran, de la carte SD et du Wi-Fi
  Hardware.ConfigEcran(tft);
  Hardware.ConfigCarteSD();
  Hardware.ConfigWiFi();

  // pinMode(PIN_AUDIO, OUTPUT);
  // digitalWrite(PIN_AUDIO, LOW);

  // Démarrer la connexion WebSocket et configurer les callbacks
  wsClient.demarrer();
  wsClient.callbackLogiqueJeu(mainJoueur, menuBas, tft);

  // Change l'état à "Attente de connexion" et affiche l'écran de connexion
  menuBas.definirEtat(EtatPartie::AttenteConnexion);
  menuBas.EcranConnexion(wsClient.estPret());
}

void loop()
{
  wsClient.actualiser();

  IndicateurConnexion.mettreAJour(tft, wsClient, menuBas); 

  int tx, ty;
  if (tft.getTouch(&tx, &ty))
  {
    ActionMenu action = menuBas.gererAction(tx, ty);

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
        menuBas.afficherActions();
        menuBas.definirEtat(EtatPartie::EnCours);
      }
      else
      {
        Serial.println("[ESP32] ⚠️ Serveur non disponible");
        menuBas.afficherMessage("Serveur hors-ligne");
        delay(5000);
        menuBas.EcranConnexion(false);
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

      mainJoueur.reinitialiser(tft, menuBas);
      menuBas.setMise(1);
      menuBas.EcranConnexion(wsClient.estPret());
      menuBas.definirEtat(EtatPartie::AttenteConnexion);

      wsClient.envoyerAction(ActionWebSocket::Rejouer);
    }

    else if (action == ActionMenu::AugmenterMise || action == ActionMenu::DiminuerMise)
    {
      menuBas.afficherMise();
    }

    delay(200); // Anti-rebond
  }
}

/*
#include <vector>
#include <WebSocketsClient.h>
#include "LGFX_ESP32.hpp"
#include "gestion_cartes.hpp"
#include "menu.hpp"
#include "audio.hpp"

#include "hardware.hpp"
#include "websocket.hpp"

LGFX tft;
MainJoueur mainJoueur;
Menu interfaceJeu(tft);
WebSocket wsClient;
HardwareConfig Hardware;
EtatReseau IndicateurConnexion;

unsigned long dernierRefresh = 0;
const unsigned long intervalleRefresh = 500;
String resultatPartie = "";

void setup()
{
  Serial.begin(115200);

  // Initialisation de l'écran, de la carte SD et du Wi-Fi
  Hardware.ConfigEcran(tft);
  interfaceJeu.InitAffichage();
  Hardware.ConfigCarteSD();
  Hardware.ConfigWiFi();

  wsClient.demarrer();
  wsClient.callbackLogiqueJeu(mainJoueur, interfaceJeu, tft);

  interfaceJeu.definirEtat(EtatPartie::AttenteConnexion);
  interfaceJeu.EcranConnexion(wsClient.estPret());
}

void loop()
{
  wsClient.actualiser();
  IndicateurConnexion.mettreAJour(tft, wsClient, interfaceJeu);

  int tx, ty;
  if (tft.getTouch(&tx, &ty))
  {
    ActionMenu action = interfaceJeu.gererAction(tx, ty);

    if (interfaceJeu.obtenirEtat() == EtatPartie::Terminee && action != ActionMenu::Rejouer)
      return;

    if (action == ActionMenu::Connexion)
    {
      if (wsClient.estPret())
      {
        Serial.println("[ESP32] ✅ Joueur prêt");

        wsClient.envoyerAction(ActionWebSocket::Pret);
        tft.fillScreen(TFT_DARKGREEN);
        interfaceJeu.afficherActions();
        interfaceJeu.definirEtat(EtatPartie::EnCours);
      }
      else
      {
        Serial.println("[ESP32] ⚠️ Serveur non disponible");
        interfaceJeu.afficherMessage("Serveur hors-ligne");
        delay(5000);
        interfaceJeu.EcranConnexion(false);
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

      mainJoueur.reinitialiser(tft, interfaceJeu);
      interfaceJeu.setMise(1);
      interfaceJeu.EcranConnexion(wsClient.estPret());
      interfaceJeu.definirEtat(EtatPartie::AttenteConnexion);

      wsClient.envoyerAction(ActionWebSocket::Rejouer);
    }
    else if (action == ActionMenu::AugmenterMise || action == ActionMenu::DiminuerMise)
    {
      interfaceJeu.afficherMise();
    }

    delay(250);
  }
}
*/
