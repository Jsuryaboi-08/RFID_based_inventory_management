#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>

#define RST_PIN 22    // Reset pin for the RFID reader
#define SS_PIN 21     // SDA pin for the RFID reader
#define INTAKE_BUTTON_PIN 4   // Pin for intake button
#define TAKEOUT_BUTTON_PIN 5  // Pin for takeout button

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

// Wi-Fi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Button states
bool intakeMode = false;
bool takeoutMode = false;

void setup() {
  Serial.begin(115200);
  SPI.begin();  
  mfrc522.PCD_Init();

  // Set up Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  // Initialize buttons
  pinMode(INTAKE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(TAKEOUT_BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  // Check if intake button is pressed
  if (digitalRead(INTAKE_BUTTON_PIN) == LOW) {
    intakeMode = true;
    takeoutMode = false;
    Serial.println("Intake mode activated.");
    delay(300); // Debounce delay
  }

  // Check if takeout button is pressed
  if (digitalRead(TAKEOUT_BUTTON_PIN) == LOW) {
    intakeMode = false;
    takeoutMode = true;
    Serial.println("Takeout mode activated.");
    delay(300); // Debounce delay
  }

  // RFID Reader in Intake Mode
  if (intakeMode) {
    scanRFID("Intake");
  }

  // RFID Reader in Takeout Mode
  if (takeoutMode) {
    scanRFID("Takeout");
  }
}

// Function to scan any RFID tag and log it
void scanRFID(String mode) {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.print(mode + " RFID Tag Detected: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();
    
    mfrc522.PICC_HaltA();
  }
}
