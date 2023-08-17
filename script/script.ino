#include <SD.h>
#include <SPI.h>
#include <LiquidCrystal.h>

// Pin configurations
const int startButtonPin = A1;
const int stopButtonPin = A2;
const int newFileButtonPin = A3;
const int voltagePin = A0;
const int currentPin = A4;
const int tempPin = A5;
const int LDRPin = 10;

// Constants for calculations
const int ACS712_TYPE = 5; // 5A version
const float VCC = 5.0; // Arduino VCC
const float ADC_SCALE = 1023.0; // 10-bit ADC

// SD file
File dataFile;

// Logging status
bool isLogging = false;

// LCD display
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup() {
  Serial.begin(9600);
  pinMode(startButtonPin, INPUT_PULLUP);
  pinMode(stopButtonPin, INPUT_PULLUP);
  pinMode(newFileButtonPin, INPUT_PULLUP);

  lcd.begin(16, 2);

  if (!SD.begin(4)) {
    Serial.println("Card failed, or not present");
  }
}

void loop() {
  double currentSum = 0;
  double voltageSum = 0;
  double temperatureSum = 0;
  double luxSum = 0;

  // Capturing data 4 times
  for (int i = 0; i < 4; i++) {
    currentSum += getCurrent();
    voltageSum += getVoltage();
    temperatureSum += getTemperature();
    luxSum += getLUX();
    delay(250); // Delay between samples
  }

  // Averaging the captured data
  double currentAvg = currentSum / 4.0;
  double voltageAvg = voltageSum / 4.0;
  double temperatureAvg = temperatureSum / 4.0;
  double luxAvg = luxSum / 4.0;

  // Displaying on LCD
  lcd.setCursor(0, 0);
  lcd.print("V:");
  lcd.print(voltageAvg);
  lcd.print(" I:");
  lcd.print(currentAvg);
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temperatureAvg);
  lcd.print(" L:");
  lcd.print(luxAvg);

  // Sending to Serial
  Serial.print("Voltage: ");
  Serial.print(voltageAvg);
  Serial.print(" Current: ");
  Serial.print(currentAvg);
  Serial.print(" Temperature: ");
  Serial.print(temperatureAvg);
  Serial.print(" Lux: ");
  Serial.println(luxAvg);

  // Logging to SD card if enabled
  if (isLogging) {
    dataFile.print(voltageAvg, 2);
    dataFile.print(",");
    dataFile.print(currentAvg, 2);
    dataFile.print(",");
    dataFile.print(temperatureAvg, 2);
    dataFile.print(",");
    dataFile.println(luxAvg, 2);
    dataFile.flush();
  }

  // Handling buttons and Serial commands
  if (digitalRead(startButtonPin) == LOW) {
    startLogging();
    delay(200);
  }
  if (digitalRead(stopButtonPin) == LOW) {
    stopLogging();
    delay(200);
  }
  if (digitalRead(newFileButtonPin) == LOW) {
    createNewFile();
    delay(200);
  }

  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    handleCommand(command);
  }

  delay(1000);
}

double getCurrent() {
  int raw = analogRead(currentPin);
  double voltage = (raw / ADC_SCALE) * VCC;
  double offsetVoltage = VCC / 2.0;
  double sensitivity = 0.185; // 185mV per A for 5A version
  return (voltage - offsetVoltage) / sensitivity;
}

double getVoltage() {
  double R1 = 15000; // 15K resistor
  double R2 = 10000; // 10K resistor
  int raw = analogRead(voltagePin);
  double Vout = (raw / ADC_SCALE) * VCC;
  return Vout / (R2 / (R1 + R2));
}

double getTemperature() {
  double val = analogRead(tempPin);
  double fenya = (val / 1023) * 5;
  double r = (5 - fenya) / fenya * 4700;
  return 1 / (log(r / 10000) / 3950 + 1 / (25 + 273.15)) - 273.15;
}

double getLUX() {
  int sensorValue = analogRead(LDRPin);
  return (sensorValue / ADC_SCALE) * 100; // 100 for scaling factor depending on your LDR
}

void startLogging() {
  isLogging = true;
  Serial.println("Logging started.");
}

void stopLogging() {
  isLogging = false;
  dataFile.close();
  Serial.println("Logging stopped.");
}

void createNewFile() {
  if (isLogging) {
    dataFile.close();
  }
  String filename = "data";
  int index = 0;
  while (SD.exists(filename + String(index) + ".csv")) {
    index++;
  }
  dataFile = SD.open(filename + String(index) + ".csv", FILE_WRITE);
  isLogging = true;
  Serial.println("New file created and logging started.");
}

void handleCommand(String command) {
  if (command == "start") {
    startLogging();
  } else if (command == "stop") {
    stopLogging();
  } else if (command == "new") {
    createNewFile();
  } else {
    Serial.println("Unknown command. Please use 'start', 'stop', or 'new'.");
  }
}
