#include "menu.hpp"

Menu::Menu() {}

void Menu::afficherEcranConnexion(LGFX &tft, bool estConnecte)
{
  tft.fillScreen(TFT_BLACK);

  // Cercle de statut de connexion
  uint16_t couleur = estConnecte ? TFT_GREEN : TFT_RED;
  tft.fillCircle(tft.width() - 20, 20, RAYON_INDICATEUR_CONNEXION, couleur);

  // Bouton Connexion
  btnConnexion.initButton<uint16_t>(
      &tft,
      tft.width() / 2, tft.height() / 2,
      L_BTN_CONNEXION, H_BTN_CONNEXION,
      COULEUR_CONTOUR, COULEUR_CONNEXION, COULEUR_TEXTE,
      "Connexion", TAILLE_TEXTE_GRAND);
  btnConnexion.drawButton();
}

void Menu::definirEtat(EtatPartie nouvelEtat) {
  etat = nouvelEtat;
}

EtatPartie Menu::obtenirEtat() const {
  return etat;
}

void Menu::afficherBoutonRejouer(LGFX &tft)
{
  btnRejouer.initButton<uint16_t>(
      &tft,
      tft.width() / 2, tft.height() / 2 + 70,
      L_BTN_ACTION, H_BTN_ACTION,
      COULEUR_CONTOUR, COULEUR_REJOUER, COULEUR_TEXTE,
      "Rejouer", TAILLE_TEXTE_GRAND);
  btnRejouer.drawButton();
}

void Menu::rafraichirEtatConnexion(LGFX &tft, bool estConnecte)
{
  // Efface l'ancien cercle (noir sur fond noir, donc pas besoin d'effacer si t'es sûr du fond)
  tft.fillCircle(tft.width() - 20, 20, RAYON_INDICATEUR_CONNEXION, TFT_BLACK);

  // Redessine selon l’état actuel
  uint16_t couleur = estConnecte ? TFT_GREEN : TFT_RED;
  tft.fillCircle(tft.width() - 20, 20, RAYON_INDICATEUR_CONNEXION, couleur);
}

void Menu::afficherMessageTemporaire(LGFX &tft, const String &message)
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

void Menu::afficherMise(LGFX &tft)
{
  int screenWidth = tft.width();

  const int yMise = tft.height() - H_MENU - 50;
  const int r = 25;
  const int centreX = screenWidth / 2;

  // Position des boutons
  int xMoins = centreX - r - 80;
  int xPlus = centreX + r + 80;

  btnMiseMoins.initButton<uint16_t>(
      &tft, xMoins, yMise,
      r * 2, r * 2,
      COULEUR_CONTOUR, TFT_RED, COULEUR_TEXTE,
      "-", TAILLE_TEXTE_GRAND, TAILLE_TEXTE_GRAND);
  btnMisePlus.initButton<uint16_t>(
      &tft, xPlus, yMise,
      r * 2, r * 2,
      COULEUR_CONTOUR, TFT_GREEN, COULEUR_TEXTE,
      "+", TAILLE_TEXTE_GRAND, TAILLE_TEXTE_GRAND);

  btnMiseMoins.drawButton();
  btnMisePlus.drawButton();

  // Affichage de la mise au centre (style bouton non interactif)
  const int largeur = L_BTN_ACTION;
  const int hauteur = H_BTN_ACTION;
  const int x = centreX - largeur / 2;
  const int y = yMise - hauteur / 2;

  // Rectangle noir avec contour (même style que les boutons)
  tft.fillRoundRect(x, y, largeur, hauteur, 8, TFT_BLACK);
  tft.drawRoundRect(x, y, largeur, hauteur, 8, COULEUR_CONTOUR);

  // Texte centré
  tft.setTextSize(TAILLE_TEXTE_GRAND);
  tft.setTextDatum(middle_center);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawNumber(miseActuelle, centreX, yMise);
}

void Menu::afficherActions(LGFX &tft)
{
  int largeur = tft.width();
  int hauteur = tft.height();

  const int espacement = 20;
  const int yCentre = hauteur - H_BTN_ACTION / 2 - 30;
  const int largeurTotale = 2 * L_BTN_ACTION + espacement;
  const int xDebut = (largeur - largeurTotale) / 2;

  // Bouton Tirer (gauche)
  btnTirer.initButton<uint16_t>(
      &tft,
      xDebut + L_BTN_ACTION / 2, yCentre,
      L_BTN_ACTION, H_BTN_ACTION,
      COULEUR_CONTOUR, COULEUR_TIRER, COULEUR_TEXTE,
      "Tirer", TAILLE_TEXTE_MOYEN, TAILLE_TEXTE_MOYEN);
  btnTirer.drawButton();

  // Bouton Rester (droite)
  btnRester.initButton<uint16_t>(
      &tft,
      xDebut + L_BTN_ACTION + espacement + L_BTN_ACTION / 2, yCentre,
      L_BTN_ACTION, H_BTN_ACTION,
      COULEUR_CONTOUR, COULEUR_RESTER, COULEUR_TEXTE,
      "Rester", TAILLE_TEXTE_MOYEN, TAILLE_TEXTE_MOYEN);
  btnRester.drawButton();

  afficherMise(tft);
}

ActionMenu Menu::gererAction(LGFX &tft, int tx, int ty)
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
