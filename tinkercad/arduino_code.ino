// --------------------------------------
// SMART AGRICULTURE MONITORING SYSTEM
// --------------------------------------
// Tinkercad Simulation:
// https://www.tinkercad.com/things/czTrZN2pkM6-stunning-maimu?sharecode=Dy9K2MuJ0RvyA5Fo0lJsNRhPeJI-zsMpfhhkVO4wJyQ
//
// Hardware: Arduino UNO R3 + Breadboard
// Sensors:
//   A0: LDR (Light Sensor)
//   A1: TMP36 (Temperature Sensor)
//   A2: POT1 (Soil Moisture Simulator)
//   A3: POT2 (Humidity Simulator)
//   A4: POT3 (Water Level Simulator)
//
// Alert LEDs:
//   D12: LED1 - Soil Moisture Alert (ON when soil < 400)
//   D11: LED2 - Humidity Alert (ON when humidity < 400)
//   D10: LED3 - Temperature Alert (ON when temp > 30°C)
//   D9:  LED4 - Water Level Alert (ON when water < 400)
// --------------------------------------

// LEDs
const int LED1 = 12;   // Soil Moisture
const int LED2 = 11;   // Humidity
const int LED3 = 10;   // Temperature
const int LED4 = 9;    // Water Level

// Sensors
const int LDR_PIN   = A0;   // Light
const int TEMP_PIN  = A1;   // TMP36
const int SOIL_PIN  = A2;   // POT1 - Soil Moisture
const int HUM_PIN   = A3;   // POT2 - Humidity
const int WATER_PIN = A4;   // POT3 - Water Level

void setup() {

  // Set LEDs as OUTPUT
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  // Start Serial Monitor
  Serial.begin(9600);

  Serial.println("Smart Agriculture Monitoring System");
  Serial.println("------------------------------------");
}

void loop() {

  // --------------------------------------
  // READ SENSOR VALUES
  // --------------------------------------

  int lightValue = analogRead(LDR_PIN);

  int tempRaw = analogRead(TEMP_PIN);

  int soilValue = analogRead(SOIL_PIN);

  int humidityValue = analogRead(HUM_PIN);

  int waterValue = analogRead(WATER_PIN);


  // --------------------------------------
  // CONVERT TMP36 VALUE TO TEMPERATURE
  // --------------------------------------

  float voltage = tempRaw * (5.0 / 1023.0);

  float temperatureC = (voltage - 0.5) * 100.0;


  // --------------------------------------
  // DISPLAY VALUES
  // --------------------------------------

  Serial.print("Light: ");
  Serial.print(lightValue);

  Serial.print(" | Temperature: ");
  Serial.print(temperatureC);
  Serial.print(" C");

  Serial.print(" | Soil Moisture: ");
  Serial.print(soilValue);

  Serial.print(" | Humidity: ");
  Serial.print(humidityValue);

  Serial.print(" | Water Level: ");
  Serial.println(waterValue);


  // --------------------------------------
  // SOIL MOISTURE
  // LED1 - D12
  // --------------------------------------

  if (soilValue < 400) {
    digitalWrite(LED1, HIGH);
  }
  else {
    digitalWrite(LED1, LOW);
  }


  // --------------------------------------
  // HUMIDITY
  // LED2 - D11
  // --------------------------------------

  if (humidityValue < 400) {
    digitalWrite(LED2, HIGH);
  }
  else {
    digitalWrite(LED2, LOW);
  }


  // --------------------------------------
  // TEMPERATURE
  // LED3 - D10
  // --------------------------------------

  if (temperatureC > 30) {
    digitalWrite(LED3, HIGH);
  }
  else {
    digitalWrite(LED3, LOW);
  }


  // --------------------------------------
  // WATER LEVEL
  // LED4 - D9
  // --------------------------------------

  if (waterValue < 400) {
    digitalWrite(LED4, HIGH);
  }
  else {
    digitalWrite(LED4, LOW);
  }


  // Wait 1 second
  delay(1000);
}
