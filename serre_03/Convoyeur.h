#pragma once

#include <Arduino.h>
#include <LedControl.h>

// ── États de la machine à état du convoyeur 
enum EtatConvoyeur {
  CONV_INACTIF,   
  CONV_ACTIF,     
  CONV_MANUEL,    
  CONV_CONSTANT   
};

class Convoyeur {
public:
 
  Convoyeur(uint8_t pinEN, uint8_t pinIN3, uint8_t pinIN4,
            uint8_t pinJoy, LedControl* matrix);

  void begin();
  void update();           

  
  void onClic();
  void onLongClic();

  int getVitesse() const { return _vitesse; }
  EtatConvoyeur getEtat() const { return _etat; }

private:
  uint8_t _pinEN, _pinIN3, _pinIN4, _pinJoy;
  LedControl* _matrix;

 EtatConvoyeur _etat;
int  _vitesse;
EtatConvoyeur _lastDisplayedEtat;
int  _lastDisplayedVitesse;

  static const int DEAD_ZONE = 15;   

  void _setMoteur(int vitesse);
  void _updateMatrice();
  int  _lireJoystick();   // retourne -100..100

  
  static const uint8_t BITMAP_INACTIF[8];
  static const uint8_t BITMAP_ACTIF[8];
  static const uint8_t BITMAP_AVANCE[8];
  static const uint8_t BITMAP_RECULE[8];
};