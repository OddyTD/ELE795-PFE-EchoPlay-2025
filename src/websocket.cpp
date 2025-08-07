#include <WiFiClient.h>
#include <cstring>

#include "LGFX_ESP32.hpp"
#include "websocket.hpp"
#include "gestion_cartes.hpp"
#include "menu.hpp"
#include "secrets.hpp"

// ========================================================
// 🔌 Connexion WebSocket
// ========================================================

void WebSocket::demarrer()
{
  if (socket.isConnected())
  {
    Serial.println("[WebSocket] Déjà connecté, on ignore la tentative.");
    return;
  }

  socket.begin(WEBSOCKET_HOST, WEBSOCKET_PORT, "/");
  socket.onEvent([this](WStype_t type, uint8_t *payload, size_t length)
                 { this->onEvent(type, payload, length); });
}

void WebSocket::actualiser()
{
  socket.loop();
}

// ========================================================
// 📤 Envoi de messages au serveur
// ========================================================

void WebSocket::envoyerAction(ActionWebSocket action)
{
  StaticJsonDocument<64> doc;

  switch (action)
  {
  case ActionWebSocket::Pret:
    doc["action"] = "pret";
    break;
  case ActionWebSocket::TirerCarte:
    doc["action"] = "tirer_carte";
    break;
  case ActionWebSocket::Rejouer:
    doc["action"] = "rejouer";
    break;
  default:
    Serial.println("[WebSocket] ⚠️ Action inconnue");
    return;
  }

  envoyer(doc);
}

void WebSocket::envoyer(const JsonDocument &doc)
{
  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));
  socket.sendTXT(buffer);
}

// ========================================================
// 🔗 Logique de jeu liée aux événements serveur
// ========================================================

void WebSocket::callbackLogiqueJeu(MainJoueur &main, Menu &menu, LGFX &tft)
{
  onMainInitiale([&](std::vector<int> indices)
                 {
    main.initialiser(indices);
    main.afficher(tft); });

  onCarteRecue([&](std::vector<int> indices)
               {
    size_t debut = main.taille();
    main.ajouter(indices);
    for (size_t i = debut; i < main.taille(); ++i) {
      Carte& c = main.getCarte(i);
      tft.fillRect(c.getPosX() - 2, c.getPosY() - 2,
                   LARGEUR_CARTE + 4, HAUTEUR_CARTE + 4, TFT_DARKGREEN);
      c.afficher(tft);
    } });

  onFinPartie([&](const String &resultat)
              {
    tft.setTextSize(5);
    tft.setTextDatum(middle_center);
    tft.setTextColor(TFT_WHITE);

    if (resultat == "Defaite") {
      tft.fillScreen(TFT_BLACK);
      tft.drawString("BRULE", tft.width() / 2, tft.height() / 2);
    } else if (resultat == "Blackjack") {
      tft.fillScreen(TFT_GOLD);
      tft.drawString("BLACKJACK", tft.width() / 2, tft.height() / 2);
    } else {
      tft.fillScreen(TFT_DARKGREY);
      tft.drawString(resultat, tft.width() / 2, tft.height() / 2);
    }

    menu.definirEtat(EtatPartie::Terminee);
    menu.afficherBoutonRejouer(); });
}

// ========================================================
// 🔧 Enregistrement des callbacks personnalisés
// ========================================================

void WebSocket::onMainInitiale(std::function<void(std::vector<int>)> cb)
{
  cbMainInitiale = cb;
}

void WebSocket::onCarteRecue(std::function<void(std::vector<int>)> cb)
{
  cbCarteRecue = cb;
}

void WebSocket::onFinPartie(std::function<void(const String &resultat)> cb)
{
  cbFinPartie = cb;
}

// ========================================================
// 📥 Réception des événements WebSocket
// ========================================================

void WebSocket::onEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_CONNECTED:
    Serial.println("[WebSocket] 🔌 Serveur détecté — en attente des joueurs");
    break;

  case WStype_TEXT:
  {
    Serial.printf("[ESP32] 📨 Reçu %.*s\n", (int)length, reinterpret_cast<char *>(payload));

    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err)
    {
      Serial.println("[WebSocket] ⚠️ Erreur de parsing JSON");
      return;
    }

    const char *action = doc["action"];
    if (!action)
      return;

    if (strcmp(action, "main_initiale") == 0 && cbMainInitiale)
    {
      std::vector<int> indices;
      for (int i : doc["cartes"].as<JsonArray>())
      {
        indices.push_back(i);
      }
      cbMainInitiale(indices);
    }
    else if (strcmp(action, "carte_envoyee") == 0 && cbCarteRecue)
    {
      std::vector<int> indices;
      for (int i : doc["cartes"].as<JsonArray>())
      {
        indices.push_back(i);
      }
      cbCarteRecue(indices);
    }
    else if (strcmp(action, "fin_partie") == 0 && cbFinPartie)
    {
      const char *resultat = doc["resultat"];
      cbFinPartie(resultat ? String(resultat) : "");
    }

    break;
  }

  case WStype_DISCONNECTED:
    Serial.println("[WebSocket] ❌ Déconnecté du serveur");
    if (length > 0 && payload)
    {
      String reason = String((char *)payload, length);
      Serial.print("[ESP32] 💬 Raison : ");
      Serial.println(reason);
    }
    break;

  default:
    break;
  }
}

/*
#include <WiFiClient.h>
#include <cstring>

#include "LGFX_ESP32.hpp"
#include "websocket.hpp"
#include "gestion_cartes.hpp"
#include "menu.hpp"
#include "secrets.hpp"

// ========================================================
// 🔌 Connexion WebSocket
// ========================================================

void WebSocket::demarrer()
{
  if (socket.isConnected())
  {
    Serial.println("[WebSocket] Déjà connecté, on ignore la tentative.");
    return;
  }

  socket.begin(WEBSOCKET_HOST, WEBSOCKET_PORT, "/");
  socket.onEvent([this](WStype_t type, uint8_t *payload, size_t length)
                 { this->onEvent(type, payload, length); });
}

void WebSocket::actualiser()
{
  socket.loop();
}

// ========================================================
// 📤 Envoi de messages au serveur
// ========================================================

void WebSocket::envoyerAction(ActionWebSocket action)
{
  StaticJsonDocument<64> doc;

  switch (action)
  {
  case ActionWebSocket::Pret:
    doc["action"] = "pret";
    break;
  case ActionWebSocket::TirerCarte:
    doc["action"] = "tirer_carte";
    break;
  case ActionWebSocket::Rejouer:
    doc["action"] = "rejouer";
    break;
  default:
    Serial.println("[WebSocket] ⚠️ Action inconnue");
    return;
  }

  envoyer(doc);
}

void WebSocket::envoyer(const JsonDocument &doc)
{
  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));
  socket.sendTXT(buffer);
}

// ========================================================
// 🔗 Logique de jeu liée aux événements serveur
// ========================================================

void WebSocket::callbackLogiqueJeu(MainJoueur &main, Menu &menu, LGFX &tft)
{
  onMainInitiale([&](std::vector<int> indices)
                 {
    main.initialiser(indices);
    main.afficher(tft); });

  onCarteRecue([&](std::vector<int> indices)
               {
    size_t debut = main.taille();
    main.ajouter(indices);
    for (size_t i = debut; i < main.taille(); ++i) {
      Carte& c = main.getCarte(i);
      tft.fillRect(c.getPosX() - 2, c.getPosY() - 2,
                   LARGEUR_CARTE + 4, HAUTEUR_CARTE + 4, TFT_DARKGREEN);
      c.afficher(tft);
    } });

  onFinPartie([&](const String &resultat)
              {
    tft.setTextSize(5);
    tft.setTextDatum(middle_center);
    tft.setTextColor(TFT_WHITE);

    if (resultat == "Defaite") {
      tft.fillScreen(TFT_BLACK);
      tft.drawString("BRULE", tft.width() / 2, tft.height() / 2);
    } else if (resultat == "Blackjack") {
      tft.fillScreen(TFT_GOLD);
      tft.drawString("BLACKJACK", tft.width() / 2, tft.height() / 2);
    } else {
      tft.fillScreen(TFT_DARKGREY);
      tft.drawString(resultat, tft.width() / 2, tft.height() / 2);
    }

    menu.definirEtat(EtatPartie::Terminee);
    menu.afficherBoutonRejouer(); });
}

// ========================================================
// 🔧 Enregistrement des callbacks personnalisés
// ========================================================

void WebSocket::onMainInitiale(std::function<void(std::vector<int>)> cb)
{
  cbMainInitiale = cb;
}

void WebSocket::onCarteRecue(std::function<void(std::vector<int>)> cb)
{
  cbCarteRecue = cb;
}

void WebSocket::onFinPartie(std::function<void(const String &resultat)> cb)
{
  cbFinPartie = cb;
}

// ========================================================
// 📥 Réception des événements WebSocket
// ========================================================

void WebSocket::onEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_CONNECTED:
    Serial.println("[WebSocket] 🔌 Serveur détecté — en attente des joueurs");
    break;

  case WStype_TEXT:
  {
    Serial.printf("[ESP32] 📨 Reçu %.*s\n", (int)length, reinterpret_cast<char *>(payload));

    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err)
    {
      Serial.println("[WebSocket] ⚠️ Erreur de parsing JSON");
      return;
    }

    const char *action = doc["action"];
    if (!action)
      return;

    if (strcmp(action, "main_initiale") == 0 && cbMainInitiale)
    {
      std::vector<int> indices;
      for (int i : doc["cartes"].as<JsonArray>())
      {
        indices.push_back(i);
      }
      cbMainInitiale(indices);
    }
    else if (strcmp(action, "carte_envoyee") == 0 && cbCarteRecue)
    {
      std::vector<int> indices;
      for (int i : doc["cartes"].as<JsonArray>())
      {
        indices.push_back(i);
      }
      cbCarteRecue(indices);
    }
    else if (strcmp(action, "fin_partie") == 0 && cbFinPartie)
    {
      const char *resultat = doc["resultat"];
      cbFinPartie(resultat ? String(resultat) : "");
    }

    break;
  }

  case WStype_DISCONNECTED:
    Serial.println("[WebSocket] ❌ Déconnecté du serveur");
    if (length > 0 && payload)
    {
      String reason = String((char *)payload, length);
      Serial.print("[ESP32] 💬 Raison : ");
      Serial.println(reason);
    }
    break;

  default:
    break;
  }
}
*/