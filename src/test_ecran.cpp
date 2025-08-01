#include "test_ecran.hpp"
#include <LovyanGFX.hpp>

void testerAffichage(LGFX& tft) {
    // Nettoie l'écran avant de commencer le test
  tft.fillScreen(TFT_BLACK);

  // Texte au centre
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);

  const char* texte = "Test Retroeclairage";
  int largeurTexte = tft.textWidth(texte);  // Calcule la largeur du texte actuel
  int posX = (tft.width() - largeurTexte) / 2;
  int posY = (tft.height() - 16) / 2;       // 16 = approx. hauteur pour text size 2

  tft.setCursor(posX, posY);
  tft.println(texte);

  // Cycle de luminosité de 0 à 100% en pas de 10%
  for (int pourcent = 0; pourcent <= 100; pourcent += 10) {
    int niveau = map(pourcent, 0, 100, 0, 255);
    tft.setBrightness(niveau);

    // Affiche l'indicateur en haut à droite
    tft.fillRect(tft.width() - 70, 0, 70, 30, TFT_BLACK);
    tft.setCursor(tft.width() - 65, 5);
    tft.printf("%d%%", pourcent);

    delay(500);
  }

  // Remet à pleine luminosité
  tft.setBrightness(255);

  // Test de couleurs d'arrière-plan : cycle à travers plusieurs couleurs
  uint16_t couleurs[] = {
  TFT_RED, TFT_GREEN, TFT_BLUE,      // Couleurs primaires
  TFT_YELLOW, TFT_CYAN, TFT_MAGENTA, // Couleurs secondaires
  TFT_WHITE, TFT_BLACK               // Blanc et noir
  };

  // Affiche successivement chaque couleur plein écran
  for (auto c : couleurs) {
    tft.fillScreen(c);  // Applique la couleur sur tout l’écran
    delay(1000);        // Pause pour bien percevoir le changement
  }

  // Affichage de formes géométriques de base
  tft.fillScreen(TFT_BLACK);                  // Nettoyer l’écran en remplissant en noir
  tft.drawRect(20, 20, 100, 60, TFT_WHITE);   // Rectangle blanc vide
  tft.fillCircle(160, 50, 30, TFT_ORANGE);    // Cercle orange rempli
  tft.drawLine(10, 120, 310, 120, TFT_GREEN); // Ligne horizontale verte
  delay(1000);                                // Pause pour visualiser les formes
}

void testerTactile(LGFX& tft) {
  // Nettoyer l’écran et configurer le texte
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);

  // Texte combiné : invitation + titre coordonnée
  const char* titre = "Touchez l'ecran - Coordonnees :";
  int titreWidth = strlen(titre) * 6 * 2; // 6 pixels par caractère * taille x2
  tft.drawString(titre, (tft.width() - titreWidth) / 2, 5); // Centré en haut

  // Boucle continue d’attente de touches
  while (true) {
    uint16_t x, y;

    if (tft.getTouch(&x, &y)) {
      // Préparer le texte des coordonnées
      char buffer[40];
      snprintf(buffer, sizeof(buffer), "X: %d   Y: %d", x, y);

      // Nettoyer la zone d'affichage des coordonnées
      tft.fillRect(0, 35, tft.width(), 25, TFT_BLACK);

      // Afficher les coordonnées détectées (centrées horizontalement)
      int coordWidth = strlen(buffer) * 6 * 2;
      tft.drawString(buffer, (tft.width() - coordWidth) / 2, 35);

      // Dessiner un cercle rouge à l'endroit touché
      tft.fillCircle(x, y, 10, TFT_RED);

      delay(500);  // Anti-rebond simple
    }
  }
}
