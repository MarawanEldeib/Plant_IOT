#include <NewPing.h>
#include <DHT.h>

#define ULTRASONIC_TRIGGER_PIN 22
#define ULTRASONIC_ECHO_PIN 23
#define ULTRASONIC_MAX_DISTANCE 350
NewPing sonar(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN, ULTRASONIC_MAX_DISTANCE);

#define DHTPIN 21 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

#define MOISTURE_SENSOR_ANALOG_PIN 35

bool pumpState = false;

#define RELAY_PIN 19
#define TANK_BASE_AREA 176.714f
#define TANK_HEIGHT 15.0f
float AmountOfWaterToUse = 1000.0f;
float previousTankWaterLevel = 0.0f;

#define BUZZER_PIN 18 // Buzzer

int ultrasonicRead;
float waterHeight;
float tankWaterLevel;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT); // Setup buzzer pin as output
}

void loop() {
  delay(2000);

  ultrasonicRead = sonar.ping_cm();
  Serial.print("Distance: ");
  Serial.print(ultrasonicRead);
  Serial.println(" cm");

  waterHeight = abs(TANK_HEIGHT - ultrasonicRead);
  tankWaterLevel = TANK_BASE_AREA * waterHeight;

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println(" %");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
  }

  int moisture = analogRead(MOISTURE_SENSOR_ANALOG_PIN);
  float moisturePercentage = (abs((float)moisture - 4095.0f) / 4095.0f) * 100.0f;

  if(moisturePercentage < 20) {
    Serial.print("Moisture: Dry, Value: ");
    Serial.println(moisturePercentage);
  } else if(moisturePercentage > 20 && moisturePercentage < 40 ) {
    Serial.print("Moisture: Moist, Value: ");
    Serial.println(moisturePercentage);
  } else if(moisturePercentage > 40 ) {
    Serial.print("Moisture: Wet, Value: ");
    Serial.println(moisturePercentage);
  }

  if((moisturePercentage < 20 && moisturePercentage > 2) && tankWaterLevel > AmountOfWaterToUse) {
    pumpState = true;
    previousTankWaterLevel = tankWaterLevel;
    float targetWaterLevel = previousTankWaterLevel - AmountOfWaterToUse;
    digitalWrite(RELAY_PIN, HIGH);
    while(tankWaterLevel > targetWaterLevel) {
      ultrasonicRead = sonar.ping_cm();
      waterHeight = abs(TANK_HEIGHT - ultrasonicRead);
      tankWaterLevel = TANK_BASE_AREA * waterHeight;
      Serial.print("Target Water Level:" );
      Serial.println(targetWaterLevel);
      Serial.print("Water Level:" );
      Serial.println(tankWaterLevel);
      delay(50);
    }
  } else {
    pumpState = false;
    digitalWrite(RELAY_PIN, LOW);
  }

  // Check if the tank is empty and sound the buzzer if it is
  if(tankWaterLevel <= 50){
    tone(BUZZER_PIN, 10000, 500);
    delay(500);
    noTone(BUZZER_PIN);
  }
  Serial.print("Water Level: ");
  Serial.println(tankWaterLevel);
}
