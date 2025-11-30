#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"

// DHT     - adafruit
// Dallas  - Miles Burton
// Onewire - Paul Stof..

#define BLUE_LED 18
#define RED_LED 19

#define MIC_PIN 2

#define DHT_PIN 32

#define CLK_PIN 23
#define DT_PIN 22

DHT dht(DHT_PIN, DHT11);

// temp 
#define ONE_WIRE_BUS 26
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long lastTemp = 0;
unsigned long lastMig = 0;
unsigned long lastPrint = 0;
int swMig=0;
float temp = 21;
int potData = 1900;
int currentStateCLK;
int lastStateCLK;

void setup() {
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(MIC_PIN, INPUT);
  pinMode(DHT_PIN, INPUT);
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
	Serial.begin(115200);
  sensors.begin();
  dht.begin();
  lastStateCLK = digitalRead(CLK_PIN);
}

void loop() {
  int micData = analogRead(MIC_PIN);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // query and show temp
  if (millis()-lastTemp > 2000) {
    sensors.requestTemperatures(); 
    temp = sensors.getTempCByIndex(0);
    Serial.printf("%f, %f, %f, %i, %i\n", temp, t, h, micData, potData);
    lastTemp = millis();
  }
  
  // pulse led depending on temp
  if (millis()-lastMig > max(-1000/(22-temp), 0.0f)) {
    lastMig = millis();
    if (swMig == 0) {
      digitalWrite(BLUE_LED, HIGH);
      swMig++;
    } else {
      digitalWrite(BLUE_LED, LOW);
      swMig = 0;
    }
  }

  // rotary encoder
  currentStateCLK = digitalRead(CLK_PIN);
  if (lastStateCLK != currentStateCLK) {
    if (digitalRead(DT_PIN) != currentStateCLK) {
      potData--;
    } else {
      potData++;
    }
    Serial.println(potData);
  }
  lastStateCLK = currentStateCLK;

  // pulsing led with mic amp
  if ( micData > potData) {
    digitalWrite(RED_LED, HIGH);
  } else {
    digitalWrite(RED_LED, LOW);
  }
}
