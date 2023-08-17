#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>

// Pins Configuration
#define START_BUTTON_PIN    0
#define STOP_BUTTON_PIN     1
#define NEW_FILE_BUTTON_PIN A2
#define LDR_SENSOR_PIN      A0
#define CURRENT_SENSOR_PIN  A1
#define VOLTAGE_SENSOR_PIN  2
#define TEMPERATURE_SENSOR_PIN A3
#define SD_CS_PIN           4
#define LCD_RS 7
#define LCD_EN 8
#define LCD_D4 5
#define LCD_D5 6
#define LCD_D6 9
#define LCD_D7 10

// LCD initialization
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// SD File
File dataFile;

// Data Logging State
bool loggingData = false;

void setup() {
  // Start Serial communication
  Serial.begin(9600);

  // Set up the buttons with internal pull-up resistors
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NEW_FILE_BUTTON_PIN, INPUT_PULLUP);

  // Initialize LCD
  lcd.begin(16, 2);

  // Initialize SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD Card Initialization Failed!");
    return;
  }
}

void loop() {
  // Check for button presses
  if (digitalRead(START_BUTTON_PIN) == LOW) {
    loggingData = true;
    startNewFile();
  }
  if (digitalRead(STOP_BUTTON_PIN) == LOW) {
    loggingData = false;
    dataFile.close();
  }
  if (digitalRead(NEW_FILE_BUTTON_PIN) == LOW && loggingData) {
    dataFile.close();
    startNewFile();
  }

  // Check for console commands
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 's') { loggingData = true; startNewFile(); }
    if (command == 'x') { loggingData = false; dataFile.close(); }
    if (command == 'n' && loggingData) { dataFile.close(); startNewFile(); }
  }

  // Log data if logging is enabled
  if (loggingData) {
    logData();
  }

  delay(200); // Short delay for button debouncing
}

void logData() {
  // Read Sensors
  int lux = analogRead(LDR_SENSOR_PIN);
  int temperature = analogRead(TEMPERATURE_SENSOR_PIN); // You may need to convert this to actual temperature
  int current = analogRead(CURRENT_SENSOR_PIN); // You may need to convert this to actual current
  int voltage = analogRead(VOLTAGE_SENSOR_PIN); // You may need to convert this to actual voltage

  // Print data to LCD
  lcd.setCursor(0, 0);
  lcd.print("L:");
  lcd.print(lux);
  lcd.print(" C:");
  lcd.print(current);
  lcd.setCursor(0, 1);
  lcd.print("V:");
  lcd.print(voltage);
  lcd.print(" T:");
  lcd.print(temperature);

  // Print data to Serial console
  Serial.print("Lux: "); Serial.print(lux);
  Serial.print(" Current: "); Serial.print(current);
  Serial.print(" Voltage: "); Serial.print(voltage);
  Serial.print(" Temperature: "); Serial.println(temperature);

  // Write data to the file
  dataFile.print(lux);
  dataFile.print(", ");
  dataFile.print(current);
  dataFile.print(", ");
  dataFile.print(voltage);
  dataFile.print(", ");
  dataFile.println(temperature);
}

void startNewFile() {
  char filename[] = "DATA00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[4] = i / 10 + '0';
    filename[5] = i % 10 + '0';
    if (!SD.exists(filename)) {
      break;
    }
  }
  dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    dataFile.println("Lux, Current, Voltage, Temperature");
  }
}
