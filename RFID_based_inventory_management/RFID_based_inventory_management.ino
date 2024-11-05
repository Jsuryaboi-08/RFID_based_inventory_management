#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>

#define RST_PIN1 22 // Reset pin for first RFID reader
#define SS_PIN1 21  // SDA pin for first RFID reader
#define RST_PIN2 19 // Reset pin for second RFID reader
#define SS_PIN2 18  // SDA pin for second RFID reader

MFRC522 mfrc522Intake(SS_PIN1, RST_PIN1);  // Create MFRC522 instance for intake
MFRC522 mfrc522Takeout(SS_PIN2, RST_PIN2); // Create MFRC522 instance for takeout

// Wi-Fi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Flags to activate/deactivate RFID readers
bool intakeMode = false;
bool takeoutMode = false;

// Array to store allowed UIDs (4 unique items)
byte allowedUIDs[4][4] = {
  {0xDE, 0xAD, 0xBE, 0xEF},
  {0x12, 0x34, 0x56, 0x78},
  {0x90, 0xAB, 0xCD, 0xEF},
  {0xFE, 0xDC, 0xBA, 0x98}
};

// Track scanned items
bool scannedItems[4] = {false, false, false, false};

void setup() {
  Serial.begin(115200);
  SPI.begin();  
  mfrc522Intake.PCD_Init();
  mfrc522Takeout.PCD_Init();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

void loop() {
  // Check for serial input
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "intake") {
      intakeMode = true;
      takeoutMode = false;
      Serial.println("Intake mode activated.");
    } 
    else if (command == "takeout") {
      intakeMode = false;
      takeoutMode = true;
      Serial.println("Takeout mode activated.");
    }
  }

  // RFID Intake Reader - Scans only specified items
  if (intakeMode) {
    scanRFID(mfrc522Intake, "Intake");
  }

  // RFID Takeout Reader - Scans only specified items
  if (takeoutMode) {
    scanRFID(mfrc522Takeout, "Takeout");
  }
}

// Function to scan RFID tags and check against allowed UIDs
void scanRFID(MFRC522& reader, String mode) {
  if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {
    for (int i = 0; i < 4; i++) {
      bool match = true;
      for (int j = 0; j < 4; j++) {
        if (reader.uid.uidByte[j] != allowedUIDs[i][j]) {
          match = false;
          break;
        }
      }
      if (match && !scannedItems[i]) {
        Serial.print(mode + " RFID Tag Detected: ");
        for (byte j = 0; j < reader.uid.size; j++) {
          Serial.print(reader.uid.uidByte[j] < 0x10 ? " 0" : " ");
          Serial.print(reader.uid.uidByte[j], HEX);
        }
        Serial.println();
        scannedItems[i] = true;  // Mark item as scanned
        break;
      }
    }
    reader.PICC_HaltA();
  }
}
