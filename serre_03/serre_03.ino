#include <Wire.h>
#include <LCD_I2C.h>
#include <OneButton.h>
#include <DHT11.h>
#include <HCSR04.h>
#include <AccelStepper.h>
#include <LedControl.h>
#include "Convoyeur.h"

// ── Branchements 
#define PIN_PHOTO A0
#define PIN_DHT 9
#define PIN_TRIG 12
#define PIN_ECHO 11
#define PIN_LED 10
#define PIN_BTN 8

// Stepper (vanne irrigation)
#define IN_1 31
#define IN_2 35
#define IN_3 33
#define IN_4 37

// Convoyeur DC
#define CONV_EN 6
#define CONV_IN3 44
#define CONV_IN4 45

// Joystick convoyeur
#define JOY_X A3
#define JOY_Y A4
#define JOY_BTN 3

// Matrice MAX7219
#define MAT_DIN 51
#define MAT_CLK 52
#define MAT_CS 53

// ── Objets
LCD_I2C lcd(0x27, 16, 2);
DHT11 dht11(PIN_DHT);
HCSR04 hc(PIN_TRIG, PIN_ECHO);
AccelStepper stepper(4, IN_1, IN_3, IN_2, IN_4);
LedControl matrix(MAT_DIN, MAT_CLK, MAT_CS, 1);

OneButton btnLCD(PIN_BTN, true);   // bouton principal (vanne / LCD)
OneButton btnConv(JOY_BTN, true);  // bouton joystick convoyeur

Convoyeur convoyeur(CONV_EN, CONV_IN3, CONV_IN4, JOY_Y, &matrix);

// ── États LCD
enum EtatLCD { BOOT,
               DHT_STATE,
               LUM_DIST_STATE,
               CALIB_STATE,
               VANNE_STATE };
EtatLCD etatLCD = BOOT;

// ── États vanne
enum EtatVanne { V_FERME,
                 V_OUVERTURE,
                 V_OUVERT,
                 V_FERMETURE,
                 V_ARRET };
EtatVanne etatVanne = V_OUVERT;  // On assume ouverte au démarrage → fermer

// ── Variables capteurs
int lightRaw = 0;
int mappedLight = 0;
int photoMin = 1023;
int photoMax = 0;
float distCm = 0.0f;
int temperature = 0;
int humidity = 0;

// ── Timing
unsigned long lastDHT = 0;
unsigned long lastLight = 0;
unsigned long lastHC = 0;
unsigned long lastSerial = 0;
unsigned long lastLCD = 0;
unsigned long stateStart = 0;
unsigned long lastLedBlink = 0;
bool ledState = false;

const unsigned long DHT_INTERVAL = 5000;
const unsigned long LIGHT_INTERVAL = 1000;
const unsigned long HC_INTERVAL = 250;
const unsigned long SERIAL_INTERVAL = 3000;
const unsigned long LCD_INTERVAL = 100;  // 10 Hz
const unsigned long BOOT_DURATION = 3000;

// ── Mapping vanne
const long VANNE_MIN = 0;
const long VANNE_MAX = 2038;

void changeStateLCD(EtatLCD s);
void afficherLCD();
void lireCapteursAsync(unsigned long now);
void gererVanne();
void gererLED(unsigned long now);
void envoyerSerie(unsigned long now);
void onClickLCD();
void onDoubleClickLCD();
void onClickConv();
void onLongClickConv();

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);

  btnLCD.attachClick(onClickLCD);
  btnLCD.attachDoubleClick(onDoubleClickLCD);

  btnConv.attachClick(onClickConv);
  btnConv.attachLongPressStart(onLongClickConv);

  lcd.begin();
  lcd.backlight();

  stepper.setMaxSpeed(500);
  stepper.setAcceleration(200);
  stepper.setCurrentPosition(VANNE_MAX);  // démarre ouvert → aller à 0
  stepper.moveTo(VANNE_MIN);
  etatVanne = V_FERMETURE;

  matrix.shutdown(0, false);
  matrix.setIntensity(0, 8);
  matrix.clearDisplay(0);

  convoyeur.begin();

  changeStateLCD(BOOT);
}


void loop() {
  unsigned long now = millis();

  btnLCD.tick();
  btnConv.tick();

  lireCapteursAsync(now);
  gererVanne();
  gererLED(now);
  convoyeur.update();
  envoyerSerie(now);

  // Rafraîchissement LCD
  if (now - lastLCD >= LCD_INTERVAL) {
    lastLCD = now;
    afficherLCD();
  }

  // Sortie automatique du BOOT
  if (etatLCD == BOOT && now - stateStart >= BOOT_DURATION) {
    changeStateLCD(DHT_STATE);
  }
}

void lireCapteursAsync(unsigned long now) {
  if (now - lastLight >= LIGHT_INTERVAL) {
    lastLight = now;
    lightRaw = analogRead(PIN_PHOTO);
    mappedLight = map(lightRaw, photoMin, photoMax, 0, 100);
    mappedLight = constrain(mappedLight, 0, 100);
  }
  if (now - lastHC >= HC_INTERVAL) {
    lastHC = now;
    distCm = hc.dist();
  }
  if (now - lastDHT >= DHT_INTERVAL) {
    lastDHT = now;
    temperature = dht11.readTemperature();
    humidity = dht11.readHumidity();
  }
}

// ── Machine à état vanne
void gererVanne() {
  stepper.run();

  switch (etatVanne) {
    case V_FERMETURE:
      if (!stepper.isRunning()) {
        etatVanne = V_FERME;
        if (etatLCD == VANNE_STATE) changeStateLCD(LUM_DIST_STATE);
      }
      break;

    case V_OUVERTURE:
      if (!stepper.isRunning()) {
        etatVanne = V_OUVERT;
      }
      break;

    case V_FERME:
      if (distCm > 0 && distCm < 20) {
        stepper.moveTo(VANNE_MAX);
        etatVanne = V_OUVERTURE;
        changeStateLCD(VANNE_STATE);
      }
      break;

    case V_OUVERT:
      if (distCm > 25) {
        stepper.moveTo(VANNE_MIN);
        etatVanne = V_FERMETURE;
        changeStateLCD(VANNE_STATE);
      }
      break;

    case V_ARRET:
      // Attend un clic (géré dans onClickLCD)
      break;
  }
}

void gererLED(unsigned long now) {
  if (etatVanne == V_OUVERTURE || etatVanne == V_FERMETURE) {
    if (now - lastLedBlink >= 100) {
      lastLedBlink = now;
      ledState = !ledState;
      digitalWrite(PIN_LED, ledState);
    }
  } else {
    // Éclairage selon luminosité
    digitalWrite(PIN_LED, mappedLight <= 30 ? HIGH : LOW);
    ledState = mappedLight <= 30;
  }
}

// ── Port série
void envoyerSerie(unsigned long now) {
  if (now - lastSerial >= SERIAL_INTERVAL) {
    lastSerial = now;
    int vannePct = map(stepper.currentPosition(), VANNE_MIN, VANNE_MAX, 0, 100);
    vannePct = constrain(vannePct, 0, 100);
    int convSpeed = convoyeur.getVitesse();

    Serial.print("Lum:");
    Serial.print(mappedLight);
    Serial.print(",Min:");
    Serial.print(map(photoMin, 0, 1023, 0, 100));
    Serial.print(",Max:");
    Serial.print(map(photoMax, 0, 1023, 0, 100));
    Serial.print(",Dist:");
    Serial.print(distCm);
    Serial.print(",T:");
    Serial.print(temperature);
    Serial.print(",H:");
    Serial.print(humidity);
    Serial.print(",Van:");
    Serial.print(vannePct);
    Serial.print(",Conv:");
    Serial.println(convSpeed);
  }
}

// ── Affichage LCD
void afficherLCD() {
  switch (etatLCD) {
    case BOOT:
      {

        break;
      }

    case DHT_STATE:
      {
        lcd.setCursor(0, 0);
        lcd.print("Temp : ");
        lcd.print(temperature);
        lcd.print(" C   ");
        lcd.setCursor(0, 1);
        lcd.print("Hum  : ");
        lcd.print(humidity);
        lcd.print(" %   ");
        break;
      }

    case LUM_DIST_STATE:
      {
        lcd.setCursor(0, 0);
        lcd.print("Lum  : ");
        lcd.print(mappedLight);
        lcd.print(" %   ");
        lcd.setCursor(0, 1);
        lcd.print("Dist : ");
        lcd.print((int)distCm);
        lcd.print(" cm  ");
        break;
      }

    case CALIB_STATE:
      {
        lcd.setCursor(0, 0);
        lcd.print("Lum min: ");
        lcd.print(map(photoMin, 0, 1023, 0, 100));
        lcd.print("  ");
        lcd.setCursor(0, 1);
        lcd.print("Lum max: ");
        lcd.print(map(photoMax, 0, 1023, 0, 100));
        lcd.print("  ");
        // Calibration continue
        int val = analogRead(PIN_PHOTO);
        if (val < photoMin) photoMin = val;
        if (val > photoMax) photoMax = val;
        break;
      }

    case VANNE_STATE:
      {
        int pct = map(stepper.currentPosition(), VANNE_MIN, VANNE_MAX, 0, 100);
        pct = constrain(pct, 0, 100);
        lcd.setCursor(0, 0);
        lcd.print("Vanne : ");
        lcd.print(pct);
        lcd.print("%   ");
        lcd.setCursor(0, 1);
        lcd.print("Etat  : ");
        switch (etatVanne) {
          case V_FERME: lcd.print("Ferme    "); break;
          case V_OUVERTURE: lcd.print("Ouverture"); break;
          case V_OUVERT: lcd.print("Ouvert   "); break;
          case V_FERMETURE: lcd.print("Fermeture"); break;
          case V_ARRET: lcd.print("Arret    "); break;
        }
        break;
      }
  }
}

// ── Changement d'état LCD
void changeStateLCD(EtatLCD s) {
  etatLCD = s;
  stateStart = millis();
  lcd.clear();

  if (s == BOOT) {
    lcd.setCursor(0, 0);
    lcd.print("Khramoff");
    lcd.setCursor(13, 1);
    lcd.print("55");
  }
  if (s == CALIB_STATE) {
    photoMin = 1023;
    photoMax = 0;
    lcd.print("Calibration...");
  }
}

// ── Callbacks bouton LCD
void onClickLCD() {
  if (etatVanne == V_OUVERTURE || etatVanne == V_FERMETURE) {
    stepper.stop();
    etatVanne = V_ARRET;
    changeStateLCD(VANNE_STATE);
    return;
  }
  if (etatVanne == V_ARRET) {
    stepper.moveTo(VANNE_MAX);
    etatVanne = V_OUVERTURE;
    changeStateLCD(VANNE_STATE);
    return;
  }
  // Navigation LCD normale
  switch (etatLCD) {
    case DHT_STATE: changeStateLCD(LUM_DIST_STATE); break;
    case LUM_DIST_STATE: changeStateLCD(VANNE_STATE); break;
    case VANNE_STATE: changeStateLCD(DHT_STATE); break;
    case CALIB_STATE: changeStateLCD(DHT_STATE); break;
    default: break;
  }
}

void onDoubleClickLCD() {
  changeStateLCD(CALIB_STATE);
}

// ── Callbacks bouton joystick
void onClickConv() {
  convoyeur.onClic();
}

void onLongClickConv() {
  convoyeur.onLongClic();
}
