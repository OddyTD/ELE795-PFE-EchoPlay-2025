#include "websocket_client.hpp"
#include "gestion_cartes.hpp"
#include <WiFiClient.h>

void WebSocketClient::demarrer(const char *host, uint16_t port)
{
  if (socket.isConnected())
  {
    Serial.println("[WebSocket] Déjà connecté, on ignore la tentative.");
    return;
  }

  socket.begin(host, port, "/");
  socket.onEvent([this](WStype_t type, uint8_t *payload, size_t length)
                 { this->onEvent(type, payload, length); });

  // Serial.printf("[ESP32] 🔌 Connexion à %s:%u\n", host, port);
}

void WebSocketClient::actualiser()
{
  socket.loop();
}

void WebSocketClient::envoyerPret()
{
  StaticJsonDocument<64> doc;
  doc["action"] = "pret";
  envoyer(doc);
}

void WebSocketClient::envoyer(const JsonDocument &doc)
{
  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));
  socket.sendTXT(buffer);
}

void WebSocketClient::onMainInitiale(std::function<void(std::vector<int>)> cb)
{
  cbMainInitiale = cb;
}

void WebSocketClient::onCarteRecue(std::function<void(std::vector<int>)> cb)
{
  cbCarteRecue = cb;
}

void WebSocketClient::onFinPartie(std::function<void(const String &resultat)> cb)
{
  cbFinPartie = cb;
}

void WebSocketClient::onEvent(WStype_t type, uint8_t* payload, size_t length)
{
  switch (type)
  {
  case WStype_CONNECTED:
    Serial.println("[WebSocket] 🔌 Serveur détecté — en attente des joueurs");
    break;

  case WStype_TEXT:
  {
    Serial.printf("[ESP32] 📨 Reçu %.*s\n", (int)length, reinterpret_cast<char*>(payload));

    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) {
      Serial.println("[WebSocket] ⚠️ Erreur de parsing JSON");
      return;
    }

    const char* action = doc["action"];
    if (!action) return;

    if (strcmp(action, "main_initiale") == 0 && cbMainInitiale) {
      std::vector<int> indices;
      for (int i : doc["cartes"].as<JsonArray>()) {
        indices.push_back(i);
      }
      cbMainInitiale(indices);
    }
    else if (strcmp(action, "carte_envoyee") == 0 && cbCarteRecue) {
      std::vector<int> indices;
      for (int i : doc["cartes"].as<JsonArray>()) {
        indices.push_back(i);
      }
      cbCarteRecue(indices);
    }
    else if (strcmp(action, "fin_partie") == 0 && cbFinPartie) {
      const char* resultat = doc["resultat"];
      cbFinPartie(resultat ? String(resultat) : "");
    }

    break;
  }

  case WStype_DISCONNECTED:
    Serial.println("[WebSocket] ❌ Déconnecté du serveur");

    if (length > 0 && payload) {
      String reason = String((char*)payload, length);
      Serial.print("[ESP32] 💬 Raison : ");
      Serial.println(reason);
    }
    break;

  default:
    break;
  }
}

