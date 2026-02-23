const int ledPins[] = {8, 9, 10, 11};  // Tableau des numéros de broches
int potentiometerPin = A1;           
int potentiometerValue = 0; 
int valeurConvertie = 0;         
int ledIndex = 0;                   // Index du DEL allume
unsigned long currentTime = 0;



void setup() {
    Serial.begin(9600);

  for (int i = 0; i < 4; i++) {
    // Initialisation des DEL en sortie
    pinMode(ledPins[i], OUTPUT); 
  } 

  pinMode(2, INPUT_PULLUP);

}

void loop() {

  int buttonState = digitalRead(2);

  potentiometerValue = analogRead(potentiometerPin);
  valeurConvertie = map(potentiometerValue, 0, 1023, 0, 20);
  readPotentiometer(buttonState);

}

void readPotentiometer(int buttonState){
  
  for(int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);
  }
  
  if(valeurConvertie <= 5){
    digitalWrite(ledPins[0], HIGH);
    if(buttonState == LOW){
      Serial.println("25% [>>>>>................]");
    }
  }
  else if(valeurConvertie <= 10){
    digitalWrite(ledPins[0], HIGH);
    digitalWrite(ledPins[1], HIGH);
    if(buttonState == LOW){
      Serial.println("50% [>>>>>>>>>>..........]");
    }
  }
  else if(valeurConvertie <= 15){
    digitalWrite(ledPins[0], HIGH);
    digitalWrite(ledPins[1], HIGH);
    digitalWrite(ledPins[2], HIGH);
    if(buttonState == LOW){
      Serial.println("75% [>>>>>>>>>>>>>>>.....]");
    }
  }
  else if(valeurConvertie <= 20){
    digitalWrite(ledPins[0], HIGH);
    digitalWrite(ledPins[1], HIGH);
    digitalWrite(ledPins[2], HIGH);
    digitalWrite(ledPins[3], HIGH);
    if(buttonState == LOW){
      Serial.println("100% [>>>>>>>>>>>>>>>>>>>>]");
    }
  }
}





