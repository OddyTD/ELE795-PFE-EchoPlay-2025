#pragma once
#include <cstdint>  // Pour uint16_t, int32_t, etc.

// Informations de connexion Wi-Fi
constexpr char* WIFI_SSID     = "No Internet";       // Remplacez par votre SSID Wi-Fi
constexpr const char* WIFI_PASSWORD = "22239698";    // Remplacez par votre mot de passe Wi-Fi

// Informations de connexion WebSocket
constexpr const char* WEBSOCKET_HOST = "192.168.1.58"; // PC
constexpr uint16_t    WEBSOCKET_PORT = 8765;
