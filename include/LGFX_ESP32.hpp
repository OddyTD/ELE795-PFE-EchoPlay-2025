#pragma once
#include <LovyanGFX.hpp>

/**********************************************************
*  Classe : LGFX                                          *
*  Hérite de : lgfx::LGFX_Device                          *
*                                                         *
*  Description :                                          *
*    Classe de configuration personnalisée pour un écran  *
*    TFT ST7796 contrôlé par un ESP32 avec interface SPI  *
*    et écran tactile capacitif GT911 en I2C.             *
**********************************************************/
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7796  _panel_instance;  // Contrôle du panneau LCD (ST7796 en SPI)
  lgfx::Bus_SPI       _bus_instance;    // Bus SPI utilisé par le panneau
  lgfx::Light_PWM     _light_instance;  // Contrôle du rétroéclairage (via PWM)
  lgfx::Touch_GT911 _touch_instance;    // Contrôle du tactile capacitif (GT911 en I2C)

public:
  /**********************************************************
  *  Constructeur : LGFX                                    *
  *  Paramètres entrants :                                  *
  *    - Aucun                                              *
  *  Retour : Aucun                                         *
  *  Description :                                          *
  *    Initialise le bus SPI, le panneau, le rétroéclairage *
  *    et le tactile capacitif pour l'affichage.            *
  **********************************************************/
  LGFX(void) {
    // -------------- Configuration du bus SPI --------------
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host     = SPI2_HOST;         // Bus SPI matériel
      //cfg.spi_mode     = 0;               // Mode SPI 0 (CPOL=0, CPHA=0)
      cfg.freq_write   = 80000000;          // Fréquence d’écriture très rapide (80 MHz)
      cfg.freq_read    = 40000000;          // Fréquence de lecture (inutile ici, mais défini)
      cfg.spi_3wire    = true;              // Utilise le mode 3 fils (MOSI, MISO, SCLK)
      //cfg.use_lock     = true;            // Active le verrouillage si plusieurs tâches accèdent au SPI
      //cfg.dma_channel  = SPI_DMA_CH_AUTO; // Canal DMA automatique (meilleure performance)
      cfg.pin_sclk     = 14;                // GPIO 14 : horloge SPI (SCLK)
      cfg.pin_mosi     = 13;                // GPIO 13 : données vers l’écran (MOSI)
      cfg.pin_miso     = 12;                // GPIO 12 : données depuis l’écran (MISO — rarement utilisé ici)
      cfg.pin_dc       = 2;                 // GPIO 2  : ligne Data/Command (obligatoire)
      _bus_instance.config(cfg);               
      _panel_instance.setBus(&_bus_instance);
    }

    // -------------- Configuration du panneau ST7796 --------------
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs           = 15;        // GPIO 15 : Chip Select (CS)
      cfg.pin_rst          = -1;        // Pas de pin de reset (utilise reset logiciel)
      //cfg.pin_busy         = -1;      // Pas de pin busy utilisée
      cfg.panel_width      = 320;       // Largeur visible réelle
      cfg.panel_height     = 480;       // Hauteur visible réelle
      //cfg.offset_x         = 0;       // Pas de décalage horizontal
      //cfg.offset_y         = 0;       // Pas de décalage vertical
      //cfg.offset_rotation  = 0;       // Pas de rotation logicielle appliquée
      //cfg.dummy_read_pixel = 8;       // Bits fictifs à ignorer lors de lecture
      //cfg.dummy_read_bits  = 1;       // Bits fictifs pour lecture RAW
      cfg.readable         = true;      // Permet la lecture des pixels
      //cfg.invert           = false;   // Pas d’inversion de couleurs nécessaire
      //cfg.rgb_order        = false;   // Couleurs dans l’ordre standard RGB
      //cfg.dlen_16bit       = false;   // Données sur 8 bits (et non 16)
      cfg.bus_shared       = true;      // Le bus SPI est partagé avec d'autres composants
      _panel_instance.config(cfg);
    }

    // -------------- Configuration du rétroéclairage PWM --------------
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl      = 27;       // GPIO 27 : Rétroéclairage (BL)
      //cfg.invert      = false;  // Signal PWM actif à l’état haut (3.3V = allumé)
      cfg.freq        = 1200;     // Fréquence du PWM (en Hz)
      cfg.pwm_channel = 7;        // Canal PWM de l’ESP32 utilisé (valeurs possibles : 0 à 7)
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

    // -------------- Configuration du tactile capacitif GT911 --------------
    {
      auto cfg = _touch_instance.config();
      cfg.pin_int = 36;           // GPIO 36 : interruption tactile (INT)
      cfg.pin_sda = 33;           // GPIO 33 : données I2C (SDA)
      cfg.pin_scl = 32;           // GPIO 32 : horloge I2C (SCL)
      cfg.i2c_addr = 0x5D;        // Adresse GT911
      cfg.i2c_port = 0;           // Bus I2C matériel numéro 0
      cfg.freq = 800000;          // Vitesse I2C (800 kHz)
      cfg.x_min = 14;             // Coordonnées min en X calibrées
      cfg.x_max = 310;            // Coordonnées max en X calibrées
      cfg.y_min = 5;              // Coordonnées min en Y calibrées
      cfg.y_max = 448;            // Coordonnées max en Y calibrées
      //cfg.offset_rotation = 0;  // Doit suivre l’orientation de l’écran
      //cfg.bus_shared = false;   // Bus I2C dédié
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    // -------------- Finaliser la configuration --------------
    setPanel(&_panel_instance);
  }
};
