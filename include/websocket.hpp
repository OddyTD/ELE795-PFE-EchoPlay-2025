#pragma once

#include <WebSocketsClient.h>
#include <vector>     // Pour std::vector
#include <functional> // Pour std::function
#include <ArduinoJson.h>

// Forward declarations pour éviter les dépendances complètes des fichiers d'en-tête LGFX_ESP32.hpp, gestion_cartes.hpp et menu.hpp
class MainJoueur;
class Menu;
class LGFX;

// Enumération des actions WebSocket pour une meilleure lisibilité
enum class ActionWebSocket
{
  Pret,
  TirerCarte,
  Rejouer
};

// Classe WebSocket : gère la connexion et les interactions avec le serveur WebSocket
class WebSocket
{
public:
  // Démarre la connexion WebSocket vers le serveur spécifié
  void demarrer();

  // Rafraîchit la connexion WebSocket
  void actualiser();

  // Vérifie si le client est prêt (connecté)
  bool estPret() { return ws.isConnected(); }

  // Envoie une action au serveur
  void envoyerAction(ActionWebSocket action);

  // Envoie un message JSON au serveur
  void envoyer(const JsonDocument &doc);

  // Connecte la logique du jeu aux événements reçus du serveur
  void callbackLogiqueJeu(MainJoueur &main, Menu &menu, LGFX &tft);

  // Enregistre les callbacks pour les événements WebSocket
  void onMainInitiale(std::function<void(std::vector<int>)> cb);
  void onCarteRecue(std::function<void(std::vector<int>)> cb);
  void onFinPartie(std::function<void(const String &resultat)> cb);

private:
  WebSocketsClient ws;

  // Callbacks pour les événements WebSocket
  std::function<void(std::vector<int>)> cbMainInitiale;
  std::function<void(std::vector<int>)> cbCarteRecue;
  std::function<void(const String &)> cbFinPartie;

  // Gère les événements WebSocket reçus
  void onEvent(WStype_t type, uint8_t *payload, size_t length);
};
