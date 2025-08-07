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
  Menu(LGFX &tft);

  int getHauteurMenu() const { return hauteurMenu; }

  int getMise() const { return miseActuelle; }
  int getMiseMax() const { return miseMaximale; }
  void setMise(int nouvelleMise) { miseActuelle = nouvelleMise; }

  void EcranConnexion(bool estConnecte);
  void LogosConnexion();
  void EtatConnexion(bool wifiOK, bool wsOK);
  void definirEtat(EtatPartie nouvelEtat);
  void afficherActions();
  void afficherMise();
  void afficherBoutonRejouer();
  void afficherMessage(const String& message);

  ActionMenu gererAction(int tx, int ty);
  EtatPartie obtenirEtat() const;

private:
  LGFX& tft;

  int hauteurMenu;
  int largeurBtnAction;
  int hauteurBtnAction;
  int largeurBtnConnexion;
  int hauteurBtnConnexion; 
  int rayonIndicateurConnexion;
  int largeurLogoWifi;
  int hauteurLogoWifi;
  int largeurLogoWS;
  int hauteurLogoWS;

  uint16_t couleurTexte;
  uint16_t couleurBtnContour;
  uint16_t couleurBtnConnexion;
  uint16_t couleurBtnRejouer;
  uint16_t couleurBtnTirer;
  uint16_t couleurBtnRester;
  uint16_t couleurBtnMisePlus;
  uint16_t couleurBtnMiseMoins;

  float texteTP;
  float texteP;
  float texteM;
  float texteG;
  float texteTG;

  LGFX_Button btnTirer;
  LGFX_Button btnRester;
  LGFX_Button btnConnexion;
  LGFX_Button btnRejouer;
  LGFX_Button btnMisePlus;
  LGFX_Button btnMiseMoins;
  
  int miseActuelle = 1;
  int miseMaximale = 100;

  EtatPartie etat = EtatPartie::AttenteConnexion;
};

/*
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

enum class EtatPartie
{
  AttenteConnexion,
  EnCours,
  Terminee
};

class Bouton
{
public:
    Bouton(LGFX &tft);

    void definir(int x, int y, int w, int h,
                 uint16_t contour, uint16_t fond, uint16_t texte,
                 const char* label,
                 float tailleTexteX,
                 float tailleTexteY);

    void dessiner();
    bool contient(int tx, int ty) const;
    void appuyer();
    void relacher();

private:
    LGFX &tft;
    LGFX_Button bouton;
};

class Menu
{
public:
  Menu(LGFX &tft);
  void InitAffichage();

  int getHauteurMenu() const { return hauteurMenu; }

  int getMise() const { return miseActuelle; }
  int getMiseMax() const { return miseMaximale; }
  void setMise(int nouvelleMise) { miseActuelle = nouvelleMise; }

  void EcranConnexion(bool estConnecte);
  void LogosConnexion();
  void EtatConnexion(bool wifiOK, bool wsOK);
  void definirEtat(EtatPartie nouvelEtat);
  void afficherActions();
  void afficherMise();
  void afficherBoutonRejouer();
  void afficherMessage(const String& message);

  ActionMenu gererAction(int tx, int ty);
  EtatPartie obtenirEtat() const;

private:
  LGFX& tft;

  int screenWidth;
  int screenHeight;

  int centreX,centreY;
  int basY, hautY;
  int gaucheX, droiteX;

  int hauteurMenu;

  int largeurBtnConnexion;
  int hauteurBtnConnexion; 
  int rayonIndicateurConnexion;
  int largeurLogoWifi;
  int hauteurLogoWifi;
  int largeurLogoWS;
  int hauteurLogoWS;
  int xLogoConnexion;
  int yLogoWifi;
  int yLogoWS;
  int xCercleConnexion;
  int yCercleWifi;
  int yCercleWS;

  int espaceBtnMise;
  int rayonBtnMise;
  int diametreBtnMise;
  int largeurAffichageMise;
  int hauteurAffichageMise;
  int largeurTotaleMise;
  int xDebutMise;
  int xBtnMiseMoins, xBtnMisePlus;
  int xAffichageMise;
  int yMise;
  int xRectMise;
  int yRectMise;

  int xBtnAction, yBtnAction;
  int espaceBtnAction;
  int largeurBtnAction;
  int hauteurBtnAction;
  int rayonBtnAction;
  
  int largeurBtnRejouer;
  int hauteurBtnRejouer; 

  uint16_t couleurTexte;
  uint16_t couleurBtnContour;
  uint16_t couleurBtnConnexion;
  uint16_t couleurBtnRejouer;
  uint16_t couleurBtnTirer;
  uint16_t couleurBtnRester;
  uint16_t couleurBtnMisePlus;
  uint16_t couleurBtnMiseMoins;

  float texteTP;
  float texteP;
  float texteM;
  float texteG;
  float texteTG;

  Bouton btnConnexion;
  Bouton btnMisePlus;
  Bouton btnMiseMoins;
  Bouton btnTirer;
  Bouton btnRester;
  Bouton btnRejouer;
  
  int miseActuelle = 1;
  int miseMaximale = 100;

  EtatPartie etat = EtatPartie::AttenteConnexion;
};
*/
