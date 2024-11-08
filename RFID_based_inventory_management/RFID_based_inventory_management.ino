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

// Button states and timing
bool intakeMode = false;
bool takeoutMode = false;
unsigned long scanStartTime = 0;
const unsigned long scanDuration = 2000;  // Scanning time window (2 seconds)

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
  // Check intake button press
  if (digitalRead(INTAKE_BUTTON_PIN) == LOW && !intakeMode && !takeoutMode) {
    intakeMode = true;
    scanStartTime = millis();
    Serial.println("Intake mode activated for 2 seconds.");
    delay(300); // Debounce delay
  }

  // Check takeout button press
  if (digitalRead(TAKEOUT_BUTTON_PIN) == LOW && !intakeMode && !takeoutMode) {
    takeoutMode = true;
    scanStartTime = millis();
    Serial.println("Takeout mode activated for 2 seconds.");
    delay(300); // Debounce delay
  }

  // Check scan timeout (2 seconds)
  if (intakeMode || takeoutMode) {
    if (millis() - scanStartTime < scanDuration) {
      // RFID Reader active during 2-second window
      scanRFID(intakeMode ? "Intake" : "Takeout");
    } else {
      // End scanning mode after 2 seconds
      intakeMode = false;
      takeoutMode = false;
      Serial.println("Scan window closed.");
    }
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

    // Exit the scan mode after a successful read
    intakeMode = false;
    takeoutMode = false;
    Serial.println("Scan complete, mode deactivated.");
  }
}
