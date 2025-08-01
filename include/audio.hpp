#include <Arduino.h>

constexpr int PIN_AUDIO = 26;  // GPIO26, per your schematic

// Nécessite un haut-parleur 8Ω (0.5–1W recommandé) branché sur la broche SPEAK
void jouerSon(int frequence = 1000, int duree = 100) {
    ledcAttachPin(PIN_AUDIO, 0);       // Canal 0
    ledcSetup(0, frequence, 8);        // 8-bit PWM, fréquence définie
    ledcWrite(0, 128);                 // 50% duty cycle
    delay(duree);                      // Durée du bip
    ledcWrite(0, 0);                   // Éteint le son
}
