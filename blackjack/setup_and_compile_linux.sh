#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   sudo ./install_deps.sh
# Installe:
#   - Toolchain C++ (g++, make, pkg-config)
#   - SDL2 + TTF + Image (pour l'UI)
#   - websocketpp + Asio (pour WebSocket sans TLS)
#   - Police DejaVu (affichage texte)
#   - Outils pratiques (rsync, ssh, sshpass, dos2unix)

if [[ "${EUID:-$(id -u)}" -ne 0 ]]; then
  echo "Veuillez lancer ce script avec sudo (ex: sudo $0)"
  exit 1
fi

apt update
apt install -y --no-install-recommends \
  g++ make pkg-config \
  libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev \
  libwebsocketpp-dev libasio-dev \
  fonts-dejavu-core \
  rsync openssh-client sshpass dos2unix

echo "✅ Dépendances installées."

echo
echo "Compilation exemples:"
echo "  # UI (Pi/Ubuntu) :"
echo "  g++ -std=c++17 -O2 blackjack_sdl_ui.cpp -o blackjack_touch \\"
echo "     -lSDL2 -lSDL2_ttf -lSDL2_image -lpthread"
echo
echo "  # Client WS C++ (localhost) :"
echo "  g++ -std=c++17 -O2 ws_test_client.cpp -o ws_test_client -lpthread"
echo
echo "Notes:"
echo "  * WebSocket sans TLS (asio_no_tls) : pas besoin de libssl-dev."
echo "  * Si besoin de corriger des fins de lignes Windows : dos2unix <fichier>."