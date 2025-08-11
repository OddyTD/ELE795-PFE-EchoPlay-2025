#!/usr/bin/env bash
set -euo pipefail

PI_HOST="echoplay@10.0.0.195"
REMOTE_DIR="~/bj"

# 1) Créer le dossier côté Pi
ssh "$PI_HOST" "mkdir -p $REMOTE_DIR"

# 2) Transférer sources + cartes
rsync -av --delete \
  blackjack_sdl_ui.cpp blackjack_engine.cpp PlayingCards/ \
  "$PI_HOST:$REMOTE_DIR/"

# 3) Compiler sur le Pi
ssh "$PI_HOST" "cd $REMOTE_DIR && g++ -std=c++17 -O2 blackjack_sdl_ui.cpp -o blackjack_touch -lSDL2 -lSDL2_ttf -lSDL2_image -pthread"

# 4) Lancer (plein écran)
ssh "$PI_HOST" "$REMOTE_DIR/blackjack_touch --fullscreen"
