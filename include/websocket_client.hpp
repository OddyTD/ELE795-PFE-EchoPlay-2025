#pragma once

#include <WebSocketsClient.h>
#include <vector>            // Pour std::vector
#include <functional>        // Pour std::function
#include <ArduinoJson.h>

class WebSocketClient {
public:
  void demarrer(const char* host, uint16_t port);
  bool estPret() { return socket.isConnected(); }
  void actualiser();
  void envoyer(const JsonDocument& doc);
  void envoyerPret();

  // Permet d’enregistrer des callbacks personnalisés
  void onMainInitiale(std::function<void(std::vector<int>)> cb);
  void onCarteRecue(std::function<void(std::vector<int>)> cb);
  void onFinPartie(std::function<void(const String& resultat)> cb);

private:
  WebSocketsClient socket;

  std::function<void(std::vector<int>)> cbMainInitiale;
  std::function<void(std::vector<int>)> cbCarteRecue;
  std::function<void(const String&)> cbFinPartie;

  void onEvent(WStype_t type, uint8_t* payload, size_t length);
};
