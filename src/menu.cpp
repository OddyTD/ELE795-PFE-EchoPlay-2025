#include <FS.h> // Pour accéder au système de fichiers
#include <SD.h> // Utilisation de la carte SD
#include "menu.hpp"

Menu::Menu(LGFX &tft) : tft(tft)
{
  // Hauteur menu
  hauteurMenu = 80;

  // Dimensions des boutons, logos et indicateurs
  largeurBtnAction = 140;
  hauteurBtnAction = 60;
  largeurBtnConnexion = 180;
  hauteurBtnConnexion = 70;
  rayonIndicateurConnexion = 12;
  largeurLogoWifi = 30;
  hauteurLogoWifi = 24;
  largeurLogoWS = 31;
  hauteurLogoWS = 24;

  // Couleurs des textes et boutons
  couleurTexte = TFT_WHITE;
  couleurBtnContour = TFT_BLACK;
  couleurBtnConnexion = TFT_DARKGREEN;
  couleurBtnRejouer = TFT_BLUE;
  couleurBtnTirer = TFT_BLUE;
  couleurBtnRester = TFT_RED;
  couleurBtnMisePlus = TFT_GREEN;
  couleurBtnMiseMoins = TFT_RED;

  // Taille du texte
  texteTP = 1.0F;
  texteP = 1.5F;
  texteM = 2.0F;
  texteG = 2.5F;
  texteTG = 3.0F;
}

void Menu::EcranConnexion(bool estConnecte)
{
  tft.fillScreen(TFT_BLACK);
  LogosConnexion();

  // Bouton Connexion
  btnConnexion.initButton<uint16_t>(
      &tft,
      tft.width() / 2, tft.height() / 2,
      largeurBtnConnexion, hauteurBtnConnexion,
      couleurBtnContour, couleurBtnConnexion, couleurTexte,
      "Connexion", texteTG);
  btnConnexion.drawButton();
}

void Menu::LogosConnexion()
{
  const int rayon = rayonIndicateurConnexion;
  const int xLogo = tft.width() - 30 - 40;

  const int y_wifi = 20;
  const int y_ws   = y_wifi + rayon * 2 + 10;

  tft.drawPngFile(SD, "/Logos/Wifi.png", xLogo, y_wifi - 12, largeurLogoWifi, hauteurLogoWifi);
  tft.drawPngFile(SD, "/Logos/WebSocket.png", xLogo, y_ws - 12, largeurLogoWS, hauteurLogoWS);
}

void Menu::EtatConnexion(bool wifiOK, bool wsOK)
{
  int x = tft.width() - 20; // Un peu plus espacé du bord
  int y_wifi = 20;
  int y_ws = y_wifi + rayonIndicateurConnexion * 2 + 5; // Cercle en dessous

  // Redessine cercles
  tft.fillCircle(x, y_wifi, rayonIndicateurConnexion, wifiOK ? TFT_GREEN : TFT_RED);
  tft.fillCircle(x, y_ws, rayonIndicateurConnexion, wsOK ? TFT_GREEN : TFT_RED);
}

void Menu::definirEtat(EtatPartie nouvelEtat) {
  etat = nouvelEtat;
}

EtatPartie Menu::obtenirEtat() const {
  return etat;
}

void Menu::afficherMessage(const String &message)
{
  // Efface la zone du bouton Connexion (adapte les dimensions selon ton layout)
  int screenWidth = tft.width();
  int screenHeight = tft.height();

  tft.fillRect(0, screenHeight - 60, screenWidth, 60, TFT_RED);

  // Affiche le message au centre de cette zone
  tft.setTextSize(2);
  tft.setTextDatum(middle_center);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(message, screenWidth / 2, screenHeight - 30);
}

void Menu::afficherMise()
{
  int screenWidth = tft.width();

  const int yMise = tft.height() - hauteurMenu - 50;
  const int r = 25;
  const int centreX = screenWidth / 2;

  // Position des boutons
  int xMoins = centreX - r - 80;
  int xPlus = centreX + r + 80;

  btnMiseMoins.initButton<uint16_t>(
      &tft, xMoins, yMise,
      r * 2, r * 2,
      couleurBtnContour, couleurBtnMiseMoins, couleurTexte,
      "-", texteTG, texteTG);
  btnMisePlus.initButton<uint16_t>(
      &tft, xPlus, yMise,
      r * 2, r * 2,
      couleurBtnContour, couleurBtnMisePlus, couleurTexte,
      "+", texteTG, texteTG);

  btnMiseMoins.drawButton();
  btnMisePlus.drawButton();

  // Affichage de la mise au centre (style bouton non interactif)
  const int largeur = largeurBtnAction;
  const int hauteur = hauteurBtnAction;
  const int x = centreX - largeur / 2;
  const int y = yMise - hauteur / 2;

  // Rectangle noir avec contour (même style que les boutons)
  tft.fillRoundRect(x, y, largeur, hauteur, 8, TFT_BLACK);
  tft.drawRoundRect(x, y, largeur, hauteur, 8, couleurBtnContour);

  // Texte centré
  tft.setTextSize(texteTG);
  tft.setTextDatum(middle_center);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawNumber(miseActuelle, centreX, yMise);
}

void Menu::afficherActions()
{
  int largeur = tft.width();
  int hauteur = tft.height();

  const int espacement = 20;
  const int yCentre = hauteur - hauteurBtnAction / 2 - 30;
  const int largeurTotale = 2 * largeurBtnAction + espacement;
  const int xDebut = (largeur - largeurTotale) / 2;

  // Bouton Tirer (gauche)
  btnTirer.initButton<uint16_t>(
      &tft,
      xDebut + largeurBtnAction / 2, yCentre,
      largeurBtnAction, hauteurBtnAction,
      couleurBtnContour, couleurBtnTirer, couleurTexte,
      "Tirer", texteM, texteM);
  btnTirer.drawButton();

  // Bouton Rester (droite)
  btnRester.initButton<uint16_t>(
      &tft,
      xDebut + largeurBtnAction + espacement + largeurBtnAction / 2, yCentre,
      largeurBtnAction, hauteurBtnAction,
      couleurBtnContour, couleurBtnRester, couleurTexte,
      "Rester", texteM, texteM);
  btnRester.drawButton();

  afficherMise();
}

void Menu::afficherBoutonRejouer()
{
  btnRejouer.initButton<uint16_t>(
      &tft,
      tft.width() / 2, tft.height() / 2 + 70,
      largeurBtnAction, hauteurBtnAction,
      couleurBtnContour, couleurBtnRejouer, couleurTexte,
      "Rejouer", texteTG);
  btnRejouer.drawButton();
}

ActionMenu Menu::gererAction(int tx, int ty)
{
  switch (etat)
  {
    case EtatPartie::AttenteConnexion:
      if (btnConnexion.contains(tx, ty))
      {
        btnConnexion.press(true);
        btnConnexion.drawButton(true);
        delay(100);
        btnConnexion.press(false);
        btnConnexion.drawButton();
        return ActionMenu::Connexion;
      }
      break;

    case EtatPartie::EnCours:
      if (btnTirer.contains(tx, ty))
      {
        btnTirer.press(true);
        btnTirer.drawButton(true);
        delay(100);
        btnTirer.press(false);
        btnTirer.drawButton();
        return ActionMenu::Draw;
      }

      if (btnRester.contains(tx, ty))
      {
        btnRester.press(true);
        btnRester.drawButton(true);
        delay(100);
        btnRester.press(false);
        btnRester.drawButton();
        return ActionMenu::Stand;
      }

      if (btnMiseMoins.contains(tx, ty))
      {
        if (miseActuelle > 1) miseActuelle--;
        return ActionMenu::DiminuerMise;
      }

      if (btnMisePlus.contains(tx, ty))
      {
        if (miseActuelle < miseMaximale) miseActuelle++;
        return ActionMenu::AugmenterMise;
      }
      break;

    case EtatPartie::Terminee:
      if (btnRejouer.contains(tx, ty))
      {
        btnRejouer.press(true);
        btnRejouer.drawButton(true);
        delay(100);
        btnRejouer.press(false);
        btnRejouer.drawButton();
        return ActionMenu::Rejouer;
      }
      break;
  }

  return ActionMenu::Rien;
}