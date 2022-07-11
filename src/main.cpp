#include <Arduino.h>
#include <Ewma.h>
#include <NoDelay.h>

#define SOIL_SENSOR_DIGITAL_PIN 2
#define SOIL_SENSOR_ANALOG_PIN A7
#define BUZZER_PIN 5
#define WATER_PUMP_PIN 4

#define SOIL_SENSOR_THRESHOLD 625 // 540
#define DELAY_AFTER_WATTERING_IN_MINUTES 60
#define DELAY_BETWEEN_MEASUREMENTS_IN_SECONDS 5
#define REQUIRED_MEASURING_SAMPLE 10

void _waterGround();
void forcePumpChange();

Ewma soilMoistureFilter(0.1);
int soilMeasuringSampleNumber = 0;
volatile int forcePumpLastUseMillis = 0;
noDelay waterGroundDelayFn(0, _waterGround); // set delay in function

void setup() {
  Serial.begin(115200);
  pinMode(SOIL_SENSOR_DIGITAL_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);
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

void loop() {
  const bool watteringRequired = digitalRead(SOIL_SENSOR_DIGITAL_PIN); // digital == 1 trzeba podlać
  const int soilMoistureRAW = analogRead(SOIL_SENSOR_ANALOG_PIN);
  const int soilMoisture = soilMoistureFilter.filter(soilMoistureRAW);
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

  if (soilMoisture > SOIL_SENSOR_THRESHOLD && watteringRequired) {
    waterGroundDelayFn.update();
  }
  else {
    delay(1000UL * DELAY_BETWEEN_MEASUREMENTS_IN_SECONDS);
  }
}
