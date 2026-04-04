#include <Wire.h>
#include <LCD_I2C.h>
#include <Joystick.h>
#include <OneButton.h>
#include <DHT11.h>

LCD_I2C lcd(0x27, 16, 2);
DHT11 dht11(7);
OneButton Button(4, true);

const int photoResistor = A0;
const int trigPin = 12;
const int echoPin = 11;
const int led = 9;
const int button = 4;

int resistor = 0;
int photoSensorMin = 100;
int photoSensorMax = 0;
int mappedLightValue = 0;
float cm = 0;
int humidity = 0;
int temperature = 0;
int state = 0;

unsigned long lastTempRead  = 0;
unsigned long lastLightRead = 0;
unsigned long lastPortWrite = 0;
const unsigned long TEMP_INTERVAL = 5000;
const unsigned long LIGHT_INTERVAL = 1000;
const unsigned long PORT_INTERVAL = 3000;

void setup() {
  Serial.begin(9600);

  pinMode(led, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Button.attachClick(click);
  Button.attachDoubleClick(doubleClick);

  lcd.begin();
  lcd.backlight();
}

void loop() {
  Button.tick();

  unsigned long now = millis();

  if (now - lastTempRead >= TEMP_INTERVAL) {
    lastTempRead = now;
    readTempHumidity();
  }

  if (now - lastLightRead >= LIGHT_INTERVAL) {
    lastLightRead = now;
    readDistance();
    mappedLightValue = map(analogRead(photoResistor), 0, 1023, 0, 100);
    mappedLightValue = constrain(mappedLightValue, photoSensorMin, photoSensorMax);
  }

  if (now - lastPortWrite >= PORT_INTERVAL) {
    lastPortWrite = now;
    printValues();
  }
  turnLedOnAndOff();

  
}

void firstScreen() {
  state = 0;
  lcd.clear();
  lcd.print("Lum : "); lcd.print(mappedLightValue); lcd.print(" %");
  lcd.setCursor(0, 1);
  lcd.print("Dist : "); lcd.print(cm); lcd.print(" cm");
}

void secondScreen() {
  state = 1;
  lcd.clear();
  lcd.print("Temp : "); lcd.print(temperature); lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print("Hum : "); lcd.print(humidity); lcd.print(" %");
}

void readTempHumidity() {
  humidity = dht11.readHumidity();
  temperature = dht11.readTemperature();
}

void readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long microSeconds = pulseIn(echoPin, HIGH);
  cm = (microSeconds * 0.0343) / 2;
}

void calibrationPhotoresistor(int durationSeconds) {
  state = 1;
  photoSensorMin = 100;
  photoSensorMax = 0;

  unsigned long phase1 = millis() + (durationSeconds / 2 * 1000);
  unsigned long phase2 = millis() + (durationSeconds * 1000);

  lcd.clear();
  lcd.print("CALIBRATION");
  lcd.setCursor(0, 1);
  lcd.print("Cover the sensor");

  long minAccum = 0;
  int  minCount = 0;

  while (millis() < phase1) {
    int val = analogRead(photoResistor);
    minAccum += val;
    minCount++;

    if (minCount >= 3) {
      int avg = minAccum / minCount;
      if (avg < photoSensorMin) photoSensorMin = avg;
      minAccum = 0;
      minCount = 0;
    }
    delay(50);
  }

  lcd.clear();
  lcd.print("CALIBRATION");
  lcd.setCursor(0, 1);
  lcd.print("Flash a light");

  long maxAccum = 0;
  int  maxCount = 0;

  while (millis() < phase2) {
    int val = analogRead(photoResistor);
    maxAccum += val;
    maxCount++;

    if (maxCount >= 3) {
      int avg = maxAccum / maxCount;
      if (avg > photoSensorMax) photoSensorMax = avg;
      maxAccum = 0;
      maxCount = 0;
    }
    delay(50);
  }

  photoSensorMin = map(photoSensorMin, 0, 1023, 0, 100);
  photoSensorMax = map(photoSensorMax, 0, 1023, 0, 100);

  lcd.clear();
  lcd.print("Min : "); lcd.print(photoSensorMin); lcd.print(" %");
  lcd.setCursor(0, 1);
  lcd.print("Max : "); lcd.print(photoSensorMax); lcd.print(" %");
  delay(2000);

  lastTempRead  = millis();
  lastLightRead = millis();
}

void turnLedOnAndOff() {
  if (mappedLightValue <= 30) {
    digitalWrite(led, HIGH);
  } else {
    digitalWrite(led, LOW);
  }
}

void printValues() {
  Serial.print("Lum : "); Serial.print("Min : "); Serial.print(photoSensorMin); Serial.print(" %  Max : "); Serial.print(photoSensorMax); Serial.println(" %");
  Serial.print("Dist : "); Serial.println(cm);
  Serial.print("Temp : "); Serial.println(temperature);
  Serial.print("Hum : "); Serial.println(humidity);
  Serial.println();
}

void click() {
  if (state == 0) {
    secondScreen();
  } else if (state == 1) {
    firstScreen();
  }
}

void doubleClick() {
  calibrationPhotoresistor(15);
}
