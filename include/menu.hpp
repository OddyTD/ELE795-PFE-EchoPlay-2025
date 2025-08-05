#pragma once

#include "LGFX_ESP32.hpp"
#include "lgfx/v1/LGFX_Button.hpp"

enum class ActionMenu
{
  Rien,
  Draw,
  Stand,
  Connexion,
  Rejouer,
  AugmenterMise,
  DiminuerMise
};

enum class EtatPartie {
  AttenteConnexion,
  EnCours,
  Terminee
};

class Menu
{
public:
  Menu();

  int getMise() const { return miseActuelle; }
  int getMiseMax() const { return miseMaximale; }
  void setMise(int nouvelleMise) { miseActuelle = nouvelleMise; }


  void afficherMise(LGFX &tft);

  void afficherActions(LGFX &tft);
  void afficherEcranConnexion(LGFX &tft, bool estConnecte);
  void definirEtat(EtatPartie nouvelEtat);
  void afficherBoutonRejouer(LGFX& tft);
  void rafraichirEtatConnexion(LGFX& tft, bool estConnecte);
  void afficherMessageTemporaire(LGFX& tft, const String& message);

  ActionMenu gererAction(LGFX &tft, int tx, int ty);
  EtatPartie obtenirEtat() const;

  inline static constexpr int H_MENU = 80;

private:
  LGFX_Button btnTirer;
  LGFX_Button btnRester;
  LGFX_Button btnConnexion;
  LGFX_Button btnRejouer;
  LGFX_Button btnMisePlus;
  LGFX_Button btnMiseMoins;
  
  int miseActuelle = 1;
  int miseMaximale = 100;

  EtatPartie etat = EtatPartie::AttenteConnexion;

  inline static constexpr int L_BTN_ACTION = 140;
  inline static constexpr int H_BTN_ACTION = 60;

  inline static constexpr int L_BTN_CONNEXION = 180;
  inline static constexpr int H_BTN_CONNEXION = 70;

  inline static constexpr uint16_t COULEUR_TEXTE = TFT_WHITE;
  inline static constexpr uint16_t COULEUR_CONTOUR = TFT_BLACK;
  inline static constexpr uint16_t COULEUR_CONNEXION = TFT_DARKGREEN;
  inline static constexpr uint16_t COULEUR_REJOUER = TFT_BLUE;
  inline static constexpr uint16_t COULEUR_TIRER = TFT_BLUE;
  inline static constexpr uint16_t COULEUR_RESTER = TFT_RED;

  inline static constexpr float TAILLE_TEXTE_TRES_PETIT = 1.0F;
  inline static constexpr float TAILLE_TEXTE_PETIT = 1.5F;
  inline static constexpr float TAILLE_TEXTE_MOYEN = 2.0F;
  inline static constexpr float TAILLE_TEXTE_GRAND = 2.5F;
  inline static constexpr float TAILLE_TEXTE_TRES_GRAND = 3.0F;

  inline static constexpr int RAYON_INDICATEUR_CONNEXION = 10;
};
