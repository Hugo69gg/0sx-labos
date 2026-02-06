const int ledPin = 13;  
long delayBlink = 250;
long delayFade = 10;
long delayOnAndOff = 1000;

void setup() { 
pinMode(ledPin, OUTPUT);
Serial.begin(9600);
}

void loop() {
  blink();
}
  
void blink(){
  Serial.println("Clignotement-2492755");
  for(int i = 0; i < 3; i++){
  digitalWrite(ledPin, HIGH);
  delay(delayBlink);
  digitalWrite(ledPin, LOW);
  delay(delayBlink);  
  fade();
  }

}

void fade(){
digitalWrite(ledPin, HIGH);
Serial.println("Variation-2492755");
for(int i = 255; i >= 0; i = i - 5){
    analogWrite(ledPin, i); // Change la luminosité de la LED
    delay(delayFade);
  }
  onAndOff();
}

void onAndOff(){
  Serial.println("Allume-2492755");

  digitalWrite(ledPin, LOW);
  delay(delayBlink);
  digitalWrite(ledPin, HIGH);
  delay(delayOnAndOff);
  digitalWrite(ledPin, LOW);
  delay(delayOnAndOff);
  blink();
}  







  


