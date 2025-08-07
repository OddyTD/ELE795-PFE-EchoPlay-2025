#include <FS.h> // Pour accéder au système de fichiers
#include <SD.h> // Utilisation de la carte SD

#include "gestion_cartes.hpp"
#include "menu.hpp"
// #include "websocket_client.hpp"  // pour le socket global

// Initialise les attributs de la carte avec l'index, position x/y et état de sélection
Carte::Carte(int index, int x, int y)
    : index(index), // Index de la carte (0 à 51)
      posX(x),      // Position x de la carte sur l'écran
      posY(y)       // Position y de la carte sur l'écran
{
}

void Carte::afficher(LGFX &tft)
{
  if (index < 0 || index >= NB_CARTES_TOTAL)
  {
    Serial.println("[Erreur] Index de carte invalide");
    return;
  }

  tft.fillRect(posX - 2, posY - 2, LARGEUR_CARTE + 4, HAUTEUR_CARTE + 4, TFT_DARKGREEN);

  String chemin = getCheminFichier();
  File file = SD.open(chemin);
  if (!file)
  {
    Serial.printf("[Erreur] Fichier %s introuvable\n", chemin.c_str());
    return;
  }

  tft.drawJpg(&file, posX, posY);
  file.close();
}

String Carte::getCheminFichier() const
{
  return "/PlayingCardsClassic/card_" + String(index) + ".jpg";
}

void Carte::setPosition(int x, int y)
{
  posX = x;
  posY = y;
}

// --- Constructeur de MainJoueur ---
MainJoueur::MainJoueur() {}

void MainJoueur::initialiser(const std::vector<int> &indices)
{
  cartes.clear();
  for (int index : indices)
  {
    cartes.emplace_back(index, 0, 0); // Position temporaire
  }
  reorganiser();
}

void MainJoueur::reinitialiser(LGFX& tft, const Menu& menuBas) {
  // Efface la zone de jeu visuellement
  tft.fillRect(0, 0, tft.width(), tft.height() - menuBas.getHauteurMenu(), TFT_DARKGREEN);

  // Vide les cartes stockées
  cartes.clear();
}


void MainJoueur::ajouter(const std::vector<int> &indices)
{
  for (int index : indices)
  {
    cartes.emplace_back(index, 0, 0); // position temporaire
  }
  reorganiser();
}

void MainJoueur::reorganiser()
{
  for (size_t i = 0; i < cartes.size(); ++i)
  {
    int col = i % NB_COLONNES;
    int row = i / NB_COLONNES;
    int x = MARGE_BORD + col * (LARGEUR_CARTE + MARGE_ENTRE);
    int y = MARGE_BORD + row * (HAUTEUR_CARTE + MARGE_ENTRE);
    cartes[i].setPosition(x, y);
  }
}

void MainJoueur::afficher(LGFX &tft)
{
  for (Carte &carte : cartes)
  {
    tft.fillRect(carte.getPosX() - 2, carte.getPosY() - 2,
                 LARGEUR_CARTE + 4, HAUTEUR_CARTE + 4, TFT_DARKGREEN);
    carte.afficher(tft);
  }
}

/*
#include <FS.h> // Pour accéder au système de fichiers
#include <SD.h> // Utilisation de la carte SD

#include "gestion_cartes.hpp"
#include "menu.hpp"
// #include "websocket_client.hpp"  // pour le socket global

// Initialise les attributs de la carte avec l'index, position x/y et état de sélection
Carte::Carte(int index, int x, int y)
    : index(index), // Index de la carte (0 à 51)
      posX(x),      // Position x de la carte sur l'écran
      posY(y)       // Position y de la carte sur l'écran
{
}

void Carte::afficher(LGFX &tft)
{
  if (index < 0 || index >= NB_CARTES_TOTAL)
  {
    Serial.println("[Erreur] Index de carte invalide");
    return;
  }

  tft.fillRect(posX - 2, posY - 2, LARGEUR_CARTE + 4, HAUTEUR_CARTE + 4, TFT_DARKGREEN);

  String chemin = getCheminFichier();
  File file = SD.open(chemin);
  if (!file)
  {
    Serial.printf("[Erreur] Fichier %s introuvable\n", chemin.c_str());
    return;
  }

  tft.drawJpg(&file, posX, posY);
  file.close();
}

String Carte::getCheminFichier() const
{
  return "/PlayingCardsClassic/card_" + String(index) + ".jpg";
}

void Carte::setPosition(int x, int y)
{
  posX = x;
  posY = y;
}

// --- Constructeur de MainJoueur ---
MainJoueur::MainJoueur() {}

void MainJoueur::initialiser(const std::vector<int> &indices)
{
  cartes.clear();
  for (int index : indices)
  {
    cartes.emplace_back(index, 0, 0); // Position temporaire
  }
  reorganiser();
}

void MainJoueur::reinitialiser(LGFX& tft, const Menu& menuBas) {
  // Efface la zone de jeu visuellement
  tft.fillRect(0, 0, tft.width(), tft.height() - menuBas.getHauteurMenu(), TFT_DARKGREEN);

  // Vide les cartes stockées
  cartes.clear();
}


void MainJoueur::ajouter(const std::vector<int> &indices)
{
  for (int index : indices)
  {
    cartes.emplace_back(index, 0, 0); // position temporaire
  }
  reorganiser();
}

void MainJoueur::reorganiser()
{
  for (size_t i = 0; i < cartes.size(); ++i)
  {
    int col = i % NB_COLONNES;
    int row = i / NB_COLONNES;
    int x = MARGE_BORD + col * (LARGEUR_CARTE + MARGE_ENTRE);
    int y = MARGE_BORD + row * (HAUTEUR_CARTE + MARGE_ENTRE);
    cartes[i].setPosition(x, y);
  }
}

void MainJoueur::afficher(LGFX &tft)
{
  for (Carte &carte : cartes)
  {
    tft.fillRect(carte.getPosX() - 2, carte.getPosY() - 2,
                 LARGEUR_CARTE + 4, HAUTEUR_CARTE + 4, TFT_DARKGREEN);
    carte.afficher(tft);
  }
}
*/