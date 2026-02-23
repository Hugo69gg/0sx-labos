const int ledPins[] = { 8, 9, 10, 11 };  // Tableau des numéros de broches
int potentiometerPin = A1;
int potentiometerValue = 0;
int valeurConvertie = 0;
int valeurAffichage = 0;
int ledIndex = 0;  // Index du DEL allume
int percent = 0;
int value = 0;
int lastButtonState = HIGH;
unsigned long currentTime = 0;
const int gradation = 20;
unsigned long previousMillis = 0;
const unsigned long interval = 20;



void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 4; i++) {
    // Initialisation des DEL en sortie
    pinMode(ledPins[i], OUTPUT);
  }

  pinMode(2, INPUT_PULLUP);
}



void loop() {
  potentiometerValue = analogRead(potentiometerPin);
  valeurConvertie = map(potentiometerValue, 0, 1023, 0, 20);
  percent = map(potentiometerValue, 0, 1023, 0, 100);
  readPotentiometer();
  diplayProgressBar();
}

void readPotentiometer() {

  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);
  }

  if (valeurConvertie <= 5) {
    digitalWrite(ledPins[0], HIGH);
  } else if (valeurConvertie <= 10) {
    digitalWrite(ledPins[0], HIGH);
    digitalWrite(ledPins[1], HIGH);
  } else if (valeurConvertie <= 15) {
    digitalWrite(ledPins[0], HIGH);
    digitalWrite(ledPins[1], HIGH);
    digitalWrite(ledPins[2], HIGH);
  } else if (valeurConvertie <= 20) {
    digitalWrite(ledPins[0], HIGH);
    digitalWrite(ledPins[1], HIGH);
    digitalWrite(ledPins[2], HIGH);
    digitalWrite(ledPins[3], HIGH);
  }
}

void diplayProgressBar() {
  int buttonState = digitalRead(2);
  value = percent / 5;
  unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) >= interval) {
    previousMillis = currentMillis;
  }





  if (lastButtonState == HIGH && buttonState == LOW) {
    Serial.print(percent);
    Serial.print("%");
    Serial.print("[");
   
    for (int i = 0; i < gradation; i++) {
      if (i < value) {
        Serial.print(">");
      } else {
        Serial.print(".");
      }
    }
    Serial.println("]");
  }


  lastButtonState = buttonState;
}
