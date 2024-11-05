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

  // RFID Intake Reader
  if (intakeMode && mfrc522Intake.PICC_IsNewCardPresent() && mfrc522Intake.PICC_ReadCardSerial()) {
    Serial.print("Intake RFID Tag: ");
    for (byte i = 0; i < mfrc522Intake.uid.size; i++) {
      Serial.print(mfrc522Intake.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522Intake.uid.uidByte[i], HEX);
    }
    Serial.println();
    mfrc522Intake.PICC_HaltA();
  }

  // RFID Takeout Reader
  if (takeoutMode && mfrc522Takeout.PICC_IsNewCardPresent() && mfrc522Takeout.PICC_ReadCardSerial()) {
    Serial.print("Takeout RFID Tag: ");
    for (byte i = 0; i < mfrc522Takeout.uid.size; i++) {
      Serial.print(mfrc522Takeout.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522Takeout.uid.uidByte[i], HEX);
    }
    Serial.println();
    mfrc522Takeout.PICC_HaltA();
  }
}
