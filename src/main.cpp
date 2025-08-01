#include <vector>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include "sd_config.hpp"
#include "LGFX_ESP32.hpp"
#include "gestion_cartes.hpp"
#include "menu.hpp"
#include "audio.hpp"
#include "wifi_credentials.hpp"
#include "websocket_client.hpp"

WebSocketsClient webSocket;
const char *WEBSOCKET_HOST = "192.168.1.58"; // PC
// const char* WEBSOCKET_HOST = "192.168.1.27";  // Laptop
const uint16_t WEBSOCKET_PORT = 8765;

LGFX tft;
MainJoueur mainJoueur;
Menu menuBas;
WebSocketClient wsClient;

unsigned long dernierRefresh = 0;
const unsigned long intervalleRefresh = 500;
String resultatPartie = "";

void setup()
{
  Serial.begin(115200);
  tft.begin();                   // Initialisation de l'écran
  tft.setRotation(1);            // écran en mode paysage
  tft.setBrightness(255);        // Rétroéclairage à pleine luminosité
  tft.fillScreen(TFT_DARKGREEN); // Fond vert foncé

  // pinMode(PIN_AUDIO, OUTPUT);
  // digitalWrite(PIN_AUDIO, LOW);

  // Initialisation de la carte SD
  if (!SDCardConfig::begin())
  {
    Serial.println("[Erreur] Carte SD non détectée");
  }
  else
  {
    Serial.println("[OK] Carte SD initialisée");
  }

  // Initialisation du WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // 🔌 Connexion WebSocket automatique
  wsClient.demarrer(WEBSOCKET_HOST, WEBSOCKET_PORT);

  // Enregistrer les callbacks
  wsClient.onMainInitiale([&](std::vector<int> indices)
                          {
    //Serial.println("[ESP32] 📥 Main initiale reçue");
    mainJoueur.initialiser(indices);
    mainJoueur.afficher(tft); });

  wsClient.onCarteRecue([&](std::vector<int> indices)
                        {
    //Serial.println("[ESP32] 🃏 Carte(s) tirée(s) reçue(s)");
    size_t debut = mainJoueur.taille();   // Nombre de cartes avant ajout
    mainJoueur.ajouter(indices);          // Ajoute et repositionne toute la main

    // Affiche seulement les nouvelles cartes (pas de flash)
    for (size_t i = debut; i < mainJoueur.taille(); ++i) {
        Carte& c = mainJoueur.getCarte(i);
        tft.fillRect(c.getPosX() - 2, c.getPosY() - 2,
                     LARGEUR_CARTE + 4, HAUTEUR_CARTE + 4, TFT_DARKGREEN);
        c.afficher(tft);
    } });

  wsClient.onFinPartie([](const String &resultat)
                       {

    tft.setTextSize(5);
    tft.setTextDatum(middle_center); // Centre le texte
    tft.setTextColor(TFT_WHITE);

    if (resultat == "Defaite") {
        tft.fillScreen(TFT_BLACK);
        tft.drawString("BUSTED", tft.width() / 2, tft.height() / 2);
    } else if (resultat == "Blackjack") {
        tft.fillScreen(TFT_GOLD); // Couleur dorée
        tft.drawString("BLACKJACK", tft.width() / 2, tft.height() / 2);
    } else {
        tft.fillScreen(TFT_DARKGREY);
        tft.drawString(resultat, tft.width() / 2, tft.height() / 2);
    } 
  
    menuBas.afficherBoutonRejouer(tft); });

  // Affiche le menu en attente
  menuBas.afficherEcranConnexion(tft, wsClient.estPret());
}

void loop()
{
  wsClient.actualiser(); // 🔄 Gère les événements WebSocket

  unsigned long maintenant = millis();

  // 🔄 Rafraîchit l’état de l’indicateur Wi-Fi toutes les secondes (avant connexion WebSocket)
  if (!wsClient.estPret() && maintenant - dernierRefresh > intervalleRefresh)
  {
    bool wifiOK = (WiFi.status() == WL_CONNECTED);
    menuBas.rafraichirEtatConnexion(tft, wifiOK);
    dernierRefresh = maintenant;
  }

  // 👆 Gestion du tactile
  int tx, ty;
  if (tft.getTouch(&tx, &ty))
  {
    ActionMenu action = menuBas.gererAction(tft, tx, ty);

    // ⛔ Bloque toute action sauf Rejouer si la partie est terminée
    if (menuBas.obtenirEtat() == EtatPartie::Terminee && action != ActionMenu::Rejouer) {
      return;
    }

    if (action == ActionMenu::Connexion)
    {
      if (wsClient.estPret())
      {
        Serial.println("[ESP32] ✅ Joueur prêt");

        wsClient.envoyerPret(); // Envoie le message {"action": "pret"}

        tft.fillScreen(TFT_DARKGREEN);
        menuBas.afficherActions(tft);
        menuBas.definirEtat(EtatPartie::EnCours); // 🟢 La partie démarre
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
      StaticJsonDocument<64> doc;
      doc["action"] = "tirer_carte";
      wsClient.envoyer(doc);
    }

    else if (action == ActionMenu::Stand && wsClient.estPret())
    {
      // TODO: Implémenter action "Stand"
    }

    else if (action == ActionMenu::Rejouer)
    {
      Serial.println("[ESP32] 🔁 Nouvelle partie demandée");

      mainJoueur.reinitialiser(tft);
      menuBas.afficherEcranConnexion(tft, wsClient.estPret());
      menuBas.definirEtat(EtatPartie::AttenteConnexion); // 🟡 En attente de nouveau départ

      // 🔌 Envoie un message au serveur
      StaticJsonDocument<64> doc;
      doc["action"] = "rejouer";
      wsClient.envoyer(doc);
    }

    delay(200); // Anti-rebond simple
  }
}

