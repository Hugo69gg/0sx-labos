const int ledPin = 13;  
const int brightness = 0;
const int fadeAmount = 5;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  allume();
  fadeAmount();
}

void allume(){
digitalWrite(ledpin, HIGH);
delay(250);
digitalWrite(ledpin, LOW);
delay(250);

}

void fadeAmount(){
  analogWrite(ledpin, brightness);
  brightness = brightness + fadeAmount;
}

