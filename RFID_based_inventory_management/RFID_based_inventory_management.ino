#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         5         // Reset pin for RFID
#define SS_PIN_1        15        // Slave Select pin for Reader 1
#define SS_PIN_2        2         // Slave Select pin for Reader 2
#define SS_PIN_3        4         // Slave Select pin for Reader 3
#define SS_PIN_4        16        // Slave Select pin for Reader 4

// Define RFID readers
MFRC522 rfid1(SS_PIN_1, RST_PIN);
MFRC522 rfid2(SS_PIN_2, RST_PIN);
MFRC522 rfid3(SS_PIN_3, RST_PIN);
MFRC522 rfid4(SS_PIN_4, RST_PIN);

// Expected tag IDs for each reader (replace with your actual tag IDs)
String expectedTag1 = "TagID1";
String expectedTag2 = "TagID2";
String expectedTag3 = "TagID3";
String expectedTag4 = "TagID4";

// Variables to track time for 5-second interval
unsigned long lastReadTime = 0;  // Store the last time tags were read
const unsigned long readInterval = 5000;  // Interval in milliseconds (5 seconds)

void setup() {
  Serial.begin(115200);
  SPI.begin();                       // Initialize SPI bus
  rfid1.PCD_Init();                  // Initialize each RFID reader
  rfid2.PCD_Init();
  rfid3.PCD_Init();
  rfid4.PCD_Init();
  Serial.println("RFID Inventory System Initialized");
}

void loop() {
  // Check if 5 seconds have passed since the last read
  if (millis() - lastReadTime >= readInterval) {
    // Update the last read time
    lastReadTime = millis();

    // Read each RFID reader
    checkReader(rfid1, expectedTag1, "Reader 1");
    checkReader(rfid2, expectedTag2, "Reader 2");
    checkReader(rfid3, expectedTag3, "Reader 3");
    checkReader(rfid4, expectedTag4, "Reader 4");
  }
}

// Function to check a single reader
void checkReader(MFRC522 &rfid, String expectedTag, String readerName) {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    Serial.println(readerName + ": No tag present");
    return;  // No tag present, skip further checking
  }

  String currentTag = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    currentTag += String(rfid.uid.uidByte[i], HEX);
  }
  currentTag.toUpperCase();

  // Check if the current tag matches the expected tag
  if (currentTag == expectedTag) {
    Serial.println(readerName + ": Correct tag present");
  } else {
    Serial.println(readerName + ": Incorrect tag detected! Object misplaced");
  }

  rfid.PICC_HaltA();  // Halt PICC
  rfid.PCD_StopCrypto1();  // Stop encryption on PCD
}
