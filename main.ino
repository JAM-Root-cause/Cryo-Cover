const int enable12 = 6;
const int input1 = 9;
const int input2 = 10;
const int enable34 = 11;
const int input3 = 12;
const int input4 = 13;

const int sensorPin = A0;

float previousTemp;
float initialTemp;
unsigned long previousTime;

bool freezeDetected = false;

bool coolingTimerStarted = false;
unsigned long coolingStartTime = 0;

unsigned long lastRapidCoolingTime = 0;

float readTemperature() {

  long sum = 0;

  for (int i = 0; i < 10; i++) {
    sum += analogRead(sensorPin);
    delay(10);
  }

  int adc = sum / 10;

  float voltage = adc * (5.0 / 1023.0);

  float temperature =
    (voltage - 0.5) / 0.01;

  return temperature;
}

void setup() {

  pinMode(enable12, OUTPUT);
  pinMode(input1, OUTPUT);
  pinMode(input2, OUTPUT);
  pinMode(enable34, OUTPUT);
  pinMode(input3, OUTPUT);
  pinMode(input4, OUTPUT);

  // Motors OFF initially
  digitalWrite(enable12, LOW);
  digitalWrite(enable34, LOW);

  Serial.begin(9600);

  previousTemp = readTemperature();
  initialTemp = readTemperature();
  previousTime = millis();
}

void loop() {

  delay(1000); // check every second for demo

  float currentTemp = readTemperature();
  unsigned long currentTime = millis();

  float deltaT = currentTemp - previousTemp;

  float deltaHours =
    (currentTime - previousTime) / 3600000.0;

  float dTdt = deltaT / deltaHours;

  Serial.print("Temperature: ");
  Serial.println(currentTemp);

  Serial.print("Warming Rate (C/hr): ");
  Serial.println(dTdt);

  previousTemp = currentTemp;

  if (dTdt < -0.15) {
    // Start timing when rapid cooling begins

    if (!coolingTimerStarted) {
      coolingTimerStarted = true;
      coolingStartTime = millis();

      Serial.println("Rapid cooling detected. Monitoring...");
    }

    // Has rapid cooling persisted for 2 seconds?
    if (millis() - coolingStartTime >= 2000) {

      freezeDetected = true;

      Serial.println("FLASH FREEZE DETECTED");

      // DC motor (Motor 34)
      analogWrite(enable34, 70);

      digitalWrite(input3, LOW);
      digitalWrite(input4, HIGH);

      // Geared motor (Motor 12)
      //roll out
      analogWrite(enable12, 120);

      digitalWrite(input1, LOW);
      digitalWrite(input2, HIGH);

      delay(3000);
      //stop
      analogWrite(enable12, 0);

      digitalWrite(input1, LOW);
      digitalWrite(input2, LOW);

      coolingTimerStarted = false;
      lastRapidCoolingTime = millis();
    }

    // check if 5 seconds have passed since warming rate stabilized

    if (freezeDetected && millis() - lastRapidCoolingTime >= 5000 && currentTemp - initialTemp >= -2.0) {
      freezeDetected = false;

      Serial.println("FLASH FREEZE OVER. RESETTING...");

      //stop sprinkling
      analogWrite(enable34, 0);

      digitalWrite(input3, LOW);
      digitalWrite(input4, LOW);

      //roll back
      analogWrite(enable12, 120);

      digitalWrite(input1, HIGH);
      digitalWrite(input2, LOW);

      delay(3000);
    }
  }
  else {
    // Cooling stopped before 2 seconds elapsed
    coolingTimerStarted = false;
  }
}
