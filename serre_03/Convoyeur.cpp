#include "Convoyeur.h"

const uint8_t Convoyeur::BITMAP_INACTIF[8] = {
  0b00000000,
  0b00000000,
  0b00000000,
  0b01111110,
  0b01111110,
  0b00000000,
  0b00000000,
  0b00000000
};

const uint8_t Convoyeur::BITMAP_ACTIF[8] = {
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00011000,
  0b00000000,
  0b00011000,
  0b00011000
};

const uint8_t Convoyeur::BITMAP_AVANCE[8] = {
  0b00000000,
  0b00010000,
  0b00110000,
  0b01111110,
  0b01111110,
  0b00110000,
  0b00010000,
  0b00000000
};

const uint8_t Convoyeur::BITMAP_RECULE[8] = {
  0b00000000,
  0b00001000,
  0b00001100,
  0b01111110,
  0b01111110,
  0b00001100,
  0b00001000,
  0b00000000
};

Convoyeur::Convoyeur(uint8_t pinEN, uint8_t pinIN3, uint8_t pinIN4,
                     uint8_t pinJoy, LedControl* matrix)
  : _pinEN(pinEN), _pinIN3(pinIN3), _pinIN4(pinIN4),
    _pinJoy(pinJoy), _matrix(matrix),
    _etat(CONV_INACTIF), _vitesse(0),
    _lastDisplayedEtat(CONV_ACTIF),
    _lastDisplayedVitesse(1)
{}

void Convoyeur::begin() {
  pinMode(_pinEN,  OUTPUT);
  pinMode(_pinIN3, OUTPUT);
  pinMode(_pinIN4, OUTPUT);

  _setMoteur(0);
  _updateMatrice();
}

void Convoyeur::update() {
  int joy = _lireJoystick();
  
  switch (_etat) {
    case CONV_INACTIF:
      _vitesse = 0;
      _setMoteur(0);
      break;

    case CONV_ACTIF:
      _vitesse = 0;
      _setMoteur(0);
      if (abs(joy) > DEAD_ZONE) {
        _etat = CONV_MANUEL;
      }
      break;

    case CONV_MANUEL:
      _vitesse = joy;
      _setMoteur(_vitesse);
      if (abs(joy) <= DEAD_ZONE) {
        _vitesse = 0;
        _setMoteur(0);
        _etat = CONV_ACTIF;
      }
      break;

    case CONV_CONSTANT:
      break;
  }

  bool dirChange = (_etat == CONV_MANUEL || _etat == CONV_CONSTANT) &&
                   ((_vitesse >= 0) != (_lastDisplayedVitesse >= 0));

  if (_etat != _lastDisplayedEtat || dirChange) {
    _updateMatrice();
    _lastDisplayedEtat    = _etat;
    _lastDisplayedVitesse = _vitesse;
  }
}

void Convoyeur::onClic() {
  switch (_etat) {
    case CONV_MANUEL:
      _etat = CONV_CONSTANT;
      break;
    case CONV_CONSTANT:
      _etat = CONV_MANUEL;
      break;
    default:
      break;
  }
}

void Convoyeur::onLongClic() {
  switch (_etat) {
    case CONV_INACTIF:
      _etat = CONV_ACTIF;
      break;
    case CONV_ACTIF:
    case CONV_MANUEL:
    case CONV_CONSTANT:
      _vitesse = 0;
      _setMoteur(0);
      _etat = CONV_INACTIF;
      break;
  }
}

void Convoyeur::_setMoteur(int vitesse) {
  vitesse = constrain(vitesse, -100, 100);

  if (vitesse > 0) {
    digitalWrite(_pinIN3, HIGH);
    digitalWrite(_pinIN4, LOW);
    analogWrite(_pinEN, map(vitesse, 0, 100, 0, 255));
  } else if (vitesse < 0) {
    digitalWrite(_pinIN3, LOW);
    digitalWrite(_pinIN4, HIGH);
    analogWrite(_pinEN, map(-vitesse, 0, 100, 0, 255));
  } else {
    digitalWrite(_pinIN3, LOW);
    digitalWrite(_pinIN4, LOW);
    analogWrite(_pinEN, 0);
  }
}

void Convoyeur::_updateMatrice() {
  const uint8_t* bmp;

  switch (_etat) {
    case CONV_INACTIF:
      bmp = BITMAP_INACTIF;
      break;
    case CONV_ACTIF:
      bmp = BITMAP_ACTIF;
      break;
    case CONV_MANUEL:
    case CONV_CONSTANT:
      bmp = (_vitesse >= 0) ? BITMAP_AVANCE : BITMAP_RECULE;
      break;
    default:
      bmp = BITMAP_INACTIF;
      break;
  }

  for (uint8_t row = 0; row < 8; row++) {
    _matrix->setRow(0, row, bmp[row]);
  }
}

int Convoyeur::_lireJoystick() {
  int raw = analogRead(_pinJoy);
  int mapped = map(raw, 0, 1023, -100, 100);
  return constrain(mapped, -100, 100);
}