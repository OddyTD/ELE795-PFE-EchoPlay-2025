#pragma once

#include <LovyanGFX.hpp>
#include <vector>        // Pour utiliser std::vector
#include <algorithm>     // Pour std::remove_if
#include <random>        // Pour std::shuffle
#include <numeric>       // Pour std::iota
#include <ArduinoJson.h> // Pour la sérialisation JSON

#include "LGFX_ESP32.hpp"

constexpr int NB_CARTES_TOTAL = 52; // Total de cartes affichées
constexpr int LARGEUR_CARTE = 71;   // Largeur d’une carte en pixels
constexpr int HAUTEUR_CARTE = 98;   // Hauteur d’une carte en pixels
constexpr int NB_COLONNES = 6;      // Nombre de cartes par ligne
// constexpr int NB_LIGNES = 3;           // Nombre de cartes par colonne
constexpr int MARGE_BORD = 3;   // Marge aux bords de l’écran
constexpr int MARGE_ENTRE = 10; // Marge entre les cartes

class Menu; // Déclaration anticipée de la classe Menu

// Classe Carte : représente une seule carte affichable
class Carte
{
public:
  Carte(int index, int x, int y);
  void afficher(LGFX &tft);
  int getPosX() const { return posX; }
  int getPosY() const { return posY; }
  void setPosition(int x, int y); // Met à jour la position de la carte

private:
  int index;
  int posX, posY;
  String getCheminFichier() const;
};

// Classe Main : gère un ensemble de cartes (la main du joueur)
class MainJoueur
{
public:
  MainJoueur();
  void initialiser(const std::vector<int> &indices); // ex: indices reçus du serveur
  void reinitialiser(LGFX& tft, const Menu& menuBas);
  void afficher(LGFX &tft);
  void ajouter(const std::vector<int> &indices);
  size_t taille() const { return cartes.size(); }
  Carte &getCarte(size_t i) { return cartes[i]; }

private:
  void reorganiser();
  std::vector<Carte> cartes;
};

/*
#pragma once

#include <LovyanGFX.hpp>
#include <vector>        // Pour utiliser std::vector
#include <algorithm>     // Pour std::remove_if
#include <random>        // Pour std::shuffle
#include <numeric>       // Pour std::iota
#include <ArduinoJson.h> // Pour la sérialisation JSON

#include "LGFX_ESP32.hpp"

class Menu;

constexpr int NB_CARTES_TOTAL = 52; // Total de cartes affichées
constexpr int LARGEUR_CARTE = 71;   // Largeur d’une carte en pixels
constexpr int HAUTEUR_CARTE = 98;   // Hauteur d’une carte en pixels
constexpr int NB_COLONNES = 6;      // Nombre de cartes par ligne
// constexpr int NB_LIGNES = 3;           // Nombre de cartes par colonne
constexpr int MARGE_BORD = 3;   // Marge aux bords de l’écran
constexpr int MARGE_ENTRE = 10; // Marge entre les cartes

// Classe Carte : représente une seule carte affichable
class Carte
{
public:
  Carte(int index, int x, int y);
  void afficher(LGFX &tft);
  int getPosX() const { return posX; }
  int getPosY() const { return posY; }
  void setPosition(int x, int y); // Met à jour la position de la carte

private:
  int index;
  int posX, posY;
  String getCheminFichier() const;
};

// Classe Main : gère un ensemble de cartes (la main du joueur)
class MainJoueur
{
public:
  MainJoueur();
  void initialiser(const std::vector<int> &indices); // ex: indices reçus du serveur
  void reinitialiser(LGFX& tft, const Menu& menuBas);
  void afficher(LGFX &tft);
  void ajouter(const std::vector<int> &indices);
  size_t taille() const { return cartes.size(); }
  Carte &getCarte(size_t i) { return cartes[i]; }

private:
  void reorganiser();
  std::vector<Carte> cartes;
};
*/
