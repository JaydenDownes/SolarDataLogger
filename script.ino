#include <SD.h>
#include <LiquidCrystal.h>

// Pin configurations
const int VOLTAGE_PIN = A0;
const int CURRENT_SENSOR_PIN = A1;
const int TEMP_SENSOR_PIN = A2;
const int LUX_SENSOR_PIN = 10; // Duinotech LDR Sensor Module
const int SD_CARD_PIN = 4;
const int BUTTON_START_PIN = A3;
const int BUTTON_STOP_PIN = A4;
const int BUTTON_NEWFILE_PIN = A5;
const int LCD_RS = 7;
const int LCD_EN = 6;
const int LCD_D4 = 5;
const int LCD_D5 = 4;
const int LCD_D6 = 3;
const int LCD_D7 = 2;

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

File dataFile;

bool isLogging = false;
int fileCounter = 0;

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_START_PIN, INPUT_PULLUP);
  pinMode(BUTTON_STOP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_NEWFILE_PIN, INPUT_PULLUP);
  lcd.begin(16, 2);

  if (!SD.begin(SD_CARD_PIN)) {
    Serial.print("SD card initialization failed. Error code: ");
    Serial.println(SD.cardErrorCode(), HEX);
    Serial.print("Data error code: ");
    Serial.println(SD.dataErrorCode(), HEX);
    lcd.print("SD Init Failed!");
    return;
  }
  lcd.print("System Ready");
}

void loop() {
  // Check for button presses
  if (digitalRead(BUTTON_START_PIN) == LOW) {
    startLogging();
  }
  if (digitalRead(BUTTON_STOP_PIN) == LOW) {
    stopLogging();
  }
  if (digitalRead(BUTTON_NEWFILE_PIN) == LOW) {
    createNewFile();
  }

  // Check for serial commands
  if (Serial.available()) {
    char command = Serial.read();
    switch (command) {
      case 's':
        startLogging();
        break;
      case 't':
        stopLogging();
        break;
      case 'n':
        createNewFile();
        break;
    }
  }

  if (isLogging) {
    logData();
  }
}

void startLogging() {
  if (!isLogging) {
    dataFile = SD.open("data" + String(fileCounter) + ".csv", FILE_WRITE);
    if (dataFile) {
      dataFile.println("Voltage,Current,Temperature,Lux");
      isLogging = true;
      lcd.print("Logging Started");
    } else {
      lcd.print("File Open Failed");
    }
  }
}

void stopLogging() {
  if (isLogging) {
    dataFile.close();
    isLogging = false;
    lcd.print("Logging Stopped");
  }
}

void createNewFile() {
  if (isLogging) {
    stopLogging();
  }
  fileCounter++;
  lcd.print("New File Created");
}

void logData() {
  // Read the voltage
  float voltage = (analogRead(VOLTAGE_PIN) / 1023.0) * 5.0 * ((15.0 + 10.0) / 10.0); // 15K and 10K voltage divider
  // Read the current using ACS712
  float current = (analogRead(CURRENT_SENSOR_PIN) - 512) * 5.0 / 1023.0 / 0.185;
  // Read the temperature using Ks0033 keyestudio Analog Temperature Sensor
  double val = analogRead(TEMP_SENSOR_PIN);
  double fenya = (val / 1023) * 5;
  double r = (5 - fenya) / fenya * 4700;
  float temperature = 1 / (log(r / 10000) / 3950 + 1 / (25 + 273.15)) - 273.15;
  // Read the LUX (please add your code for LUX calculation)
  float lux = analogRead(LUX_SENSOR_PIN); // Placeholder for LUX reading

  // Log to SD card
  dataFile.print(voltage); dataFile.print(",");
  dataFile.print(current); dataFile.print(",");
  dataFile.print(temperature); dataFile.print(",");
  dataFile.println(lux);

  // Print to serial
  Serial.print(voltage); Serial.print("\t");
  Serial.print(current); Serial.print("\t");
  Serial.print(temperature); Serial.print("\t");
  Serial.println(lux);

  // Display on LCD
  lcd.setCursor(0, 0);
  lcd.print("V: "); lcd.print(voltage);
  lcd.print(" C: "); lcd.print(current);
  lcd.setCursor(0, 1);
  lcd.print("T: "); lcd.print(temperature);
  lcd.print(" L: "); lcd.print(lux);
}
