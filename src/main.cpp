#include <Arduino.h>
#include <Ewma.h>
#include <NoDelay.h>
#include "AppWifi.h"

#define SOIL_SENSOR_DIGITAL_PIN D0
#define SOIL_SENSOR_ANALOG_PIN A0
#define BUZZER_PIN D1
#define WATER_PUMP_PIN D2

#define SOIL_SENSOR_THRESHOLD 625 // 540
#define DELAY_AFTER_WATTERING_IN_MINUTES 60
#define DELAY_BETWEEN_MEASUREMENTS_IN_SECONDS 5
#define DELAY_BETWEEN_SHEET_LOG_IN_MINUTES 1
#define REQUIRED_MEASURING_SAMPLE 10

void _waterGround();
void _sheetLog();
void forcePumpChange();

bool watteringRequired;
int soilMoistureRAW, soilMoisture;

Ewma soilMoistureFilter(0.1);
int soilMeasuringSampleNumber = 0;
volatile int forcePumpLastUseMillis = 0;
noDelay waterGroundDelayFn(0, _waterGround); // set delay in function
noDelay sheetLogDelayFn(1000UL * 60 * DELAY_BETWEEN_SHEET_LOG_IN_MINUTES, _sheetLog, true);

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SOIL_SENSOR_DIGITAL_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);

  AppWiFi::initWiFi();
  AppWiFi::checkWiFiConnection();

  Serial.println("Starting...\n");
  delay(2000);
  digitalWrite(LED_BUILTIN, HIGH);
}

void _waterGround() {
  // notifies the user that the water is being pumped
  Serial.println("Watering the ground");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
  delay(2000);

  digitalWrite(WATER_PUMP_PIN, HIGH);
  delay(4000); // 6000 == 100ml, 1L/min
  digitalWrite(WATER_PUMP_PIN, LOW);

  delay(4000); // pause for water to drain
  digitalWrite(WATER_PUMP_PIN, HIGH);
  delay(3000);
  digitalWrite(WATER_PUMP_PIN, LOW);

  delay(2000); // pause for water to drain
  digitalWrite(WATER_PUMP_PIN, HIGH);
  delay(3000);
  digitalWrite(WATER_PUMP_PIN, LOW);

  // required delay after wattering in minutes
  waterGroundDelayFn.setdelay(1000UL * 60 * DELAY_AFTER_WATTERING_IN_MINUTES);
}

void _sheetLog() {
  AppWiFi::sendDataToGoogleScript("appendRow", "" + String(watteringRequired == 0 ? "" : "1") + ";" + String(soilMoistureRAW) + ";" + String(soilMoisture));
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);
  watteringRequired = digitalRead(SOIL_SENSOR_DIGITAL_PIN); // digital == 1 trzeba podlać
  soilMoistureRAW = analogRead(SOIL_SENSOR_ANALOG_PIN);
  soilMoisture = soilMoistureFilter.filter(soilMoistureRAW);
  Serial.print("Digital: ");
  Serial.print(watteringRequired);
  Serial.print(", Analog: ");
  Serial.print(soilMoisture);
  Serial.print(", Analog raw: ");
  Serial.println(soilMoistureRAW);

  if (soilMeasuringSampleNumber < REQUIRED_MEASURING_SAMPLE) {
    Serial.print("soilMeasuringSampleNumber: ");
    Serial.println(soilMeasuringSampleNumber);
    soilMeasuringSampleNumber++;
    delay(2000);
    return;
  }

  digitalWrite(LED_BUILTIN, HIGH);

  if (soilMoisture > SOIL_SENSOR_THRESHOLD && watteringRequired) {
    AppWiFi::sendDataToGoogleScript("appendRow", "" + String(watteringRequired == 0 ? "" : "1") + ";" + String(soilMoistureRAW) + ";" + String(soilMoisture) + ";100");
    waterGroundDelayFn.update();
  }
  else {
    sheetLogDelayFn.update();
    delay(1000UL * DELAY_BETWEEN_MEASUREMENTS_IN_SECONDS);
  }
}
