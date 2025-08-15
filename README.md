
# 🎮 ESP32 Tactile Card Game UI

[![Status](https://img.shields.io/badge/status-working-brightgreen.svg)]()
[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/OddyTD/ELE795-PFE-EchoPlay-2025)
[![Platform](https://img.shields.io/badge/platform-ESP32-blue.svg)]()
![PlatformIO](https://img.shields.io/badge/made%20with-PlatformIO-orange)
![Repo size](https://img.shields.io/github/repo-size/OddyTD/ELE795-PFE-EchoPlay-2025)
![Last commit](https://img.shields.io/github/last-commit/OddyTD/ELE795-PFE-EchoPlay-2025)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

---

## 📊 Statistiques GitHub

![Langs](https://github-readme-stats.vercel.app/api/top-langs/?username=OddyTD&repo=ELE795-PFE-EchoPlay-2025&layout=compact)

---

## 📚 Sommaire

- [Fonctionnalités](#-fonctionnalités)
- [Matériel requis](#-matériel-requis)
- [Installation](#-installation)
- [Utilisation](#-utilisation)
- [Architecture](#-architecture)
- [Exemple de log](#-exemple-de-log-côté-esp32)
- [Auteur](#-auteur)
- [Licence](#-licence)
- [Capture d’écran](#capture-décran-optionnel)
- [Améliorations futures](#-améliorations-futures)

---

## 📦 Fonctionnalités

- Affichage des cartes à jouer (JPEG depuis carte SD)
- Interface tactile avec boutons "Play" et "Cancel"
- Sélection multiple de cartes par appui tactile
- Communication en temps réel via WebSocket
- Réception dynamique d'une main de cartes depuis un serveur Python
- Réorganisation automatique des cartes après sélection ou suppression

---

## 🛠 Matériel requis

- ESP32 (ex. ESP32-3248S035C)
- Écran SPI compatible (ex. 3.5” TFT avec GT911 touch)
- Carte SD contenant les cartes JPEG [PlayingCards](/data/PlayingCards/71x98)
- Serveur WebSocket (fourni dans [server.py](/server.py))

---

## 🚀 Installation

1. **Préparer la carte SD** :
    - Créer un dossier `/PlayingCardsClassic/`
    - Y copier les fichiers `card_0.jpg` à `card_51.jpg` (nommage indexé)
   
2. **Installer les dépendances avec PlatformIO** :
    - Ouvrir le projet dans VSCode avec PlatformIO
    - S'assurer que les bibliothèques suivantes sont incluses :
        - `LovyanGFX`
        - `ArduinoJson`
        - `WebSockets`

3. **Configurer les identifiants Wi-Fi** :
   Créer un fichier `wifi_credentials.hpp` :
   ```cpp
   #define WIFI_SSID "TonSSID"
   #define WIFI_PASSWORD "TonMotDePasse"
   ```

4. **Flasher l'ESP32** :
    - Connecter le module et uploader via PlatformIO

5. **Installer le serveur WebSocket Python** :
    ```python
    pip install websockets
    python server.py
    ```
---

## 🖱 Utilisation
- Les cartes sont affichées dès que le serveur distribue une main.
- Touchez une carte pour la sélectionner/désélectionner (bordure rouge visible).
- Le menu bas affiche deux boutons :
        - Play : envoie les cartes sélectionnées au serveur
        - Cancel : désélectionne toutes les cartes
- Les cartes jouées disparaissent et la main est réorganisée.

---

## 🧠 Architecture
```text
ESP32
├── LovyanGFX (affichage écran)
├── GT911 (tactile capacitif)
├── Carte SD (JPEG des cartes)
├── WebSocketClient ↔️ server.py
└── Interface en C++ : gestion_cartes, boîte_dialogue, websocket_client
```

---

## 🧪 Exemple de log côté ESP32
[ESP32] ✅ Connected to WebSocket server
[ESP32] 📨 Received: { "action": "distribuer_cartes", "cartes": [12, 27, 5, ...] }
[ESP32] ✅ ACK envoyé: cartes_reçues

---

## 👤 Auteur
Tung Do
Étudiant à l'ÉTS, projet personnel de jeu de cartes interactif

---

## 📄 Licence
Ce projet est distribué sous la licence [MIT](LICENSE).  
Vous êtes libre de l’utiliser, le modifier et le distribuer sous certaines conditions.

---

## Capture d’écran (optionnel)
Ajoute ici une ou deux images du rendu à l’écran :
![Aperçu](docs/demo_ui.jpg)

---

## 💡 Améliorations futures
- Animation des cartes
- Retour haptique ou sonore
- Mode multijoueur
- Support d’un deck personnalisé