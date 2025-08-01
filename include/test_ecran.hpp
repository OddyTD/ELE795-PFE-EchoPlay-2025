// test_ecran.hpp
#pragma once

#include "LGFX_ESP32.hpp"

/**********************************************************
*  Fonction : testerAffichage                             *
*  Paramètres entrants :                                  *
*     - LGFX& tft : instance de l'écran                   *
*  Retour :                                               *
*     - void                                              *
*  Description :                                          *
*     Teste les fonctionnalités d'affichage de l'écran :  *
*     cycle de luminosité, affichage de couleurs et       *
*     formes de base.                                     *
**********************************************************/
void testerAffichage(LGFX& tft);

/**********************************************************
*  Fonction : testerTactile                               *
*  Paramètres entrants :                                  *
*     - LGFX& tft : instance de l'écran                   *
*  Retour :                                               *
*     - void                                              *
*  Description :                                          *
*     Teste le tactile capacitif de l'écran en dessinant  *
*     des cercles aux points de contact et en affichant   *
*     les coordonnées.                                    *
**********************************************************/
void testerTactile(LGFX& tft);
