#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>

// Pin definitions
#define SS_PIN    5  
#define RST_PIN   22
#define SCK_PIN   18
#define MISO_PIN  19
#define MOSI_PIN  23
#define IN_BUTTON 25
#define OUT_BUTTON 26
#define BUZZER    27

// WiFi credentials
const char* ssid = "12345";
const char* password = "surya12345";

// API endpoint
const char* serverName = "https://deafening-gnu-222.convex.site/insertItem";

// NTP Server settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;  // GMT +5:30 for IST
const int   daylightOffset_sec = 0;

MFRC522 rfid(SS_PIN, RST_PIN);

// Product structure
struct Product {
  const char* name;
  const char* categoryId;
  int quantity;
  int price;
  const char* rfid;
  bool isIn;
};

// Product database
const Product products[] = {
  {"IPHONE_16_PR0_MAX", "jd7d6qtr650dwnqydav9yn7khn742c2y", 5, 144900, "D3 CF 2D DA", true},
  {"REALME_13_PRO", "jd7d6qtr650dwnqydav9yn7khn742c2y", 5, 26999, "13 FC E4 D9", true},
  {"DURACELL_2A", "jd7bc9fk3craa2hb5pzvcj1ptx742pkx", 10, 20, "3D 03 34 02", true},
  {"EVEREADY_2A", "jd7bc9fk3craa2hb5pzvcj1ptx742pkx", 10, 18, "CC 8E 31 02", true},
  {"TATA_KUSHAQ", "jd7dhe676z0ft7dwv5z8nak69x743n20", 1, 1100000, "43 4B 44 E2", true},
  {"MARUTI_ALTO_K10", "jd7dhe676z0ft7dwv5z8nak69x743n20", 1, 400000, "33 20 4F E2", true}
};

bool buttonPressed = false;
bool waitingForCard = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(IN_BUTTON, INPUT_PULLUP);
  pinMode(OUT_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  
  // Initialize RFID
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  rfid.PCD_Init();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Initialize time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

String getFormattedUID() {
  String cardID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) cardID += "0";
    cardID += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) cardID += " ";
  }
  cardID.toUpperCase();
  return cardID;
}

void buzzerBeep() {
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
}

void sendToAPI(const Product& product, bool isIn, time_t intime, time_t outtime) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"name\":\"" + String(product.name) + 
                        "\",\"categoryId\":\"" + String(product.categoryId) + 
                        "\",\"quantity\":" + String(product.quantity) + 
                        ",\"price\":" + String(product.price) + 
                        ",\"rfid\":\"" + String(product.rfid) + 
                        "\",\"intime\":" + String(intime) + 
                        ",\"outtime\":" + String(outtime) + 
                        ",\"isIn\":" + String(isIn ? "true" : "false") + "}";

    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
    }
    
    http.end();
  }
}

void loop() {
  // Check buttons
  if (!buttonPressed && (digitalRead(IN_BUTTON) == LOW || digitalRead(OUT_BUTTON) == LOW)) {
    buttonPressed = true;
    waitingForCard = true;
    bool isIn = (digitalRead(IN_BUTTON) == LOW);
    Serial.println(isIn ? "IN button pressed" : "OUT button pressed");
  }

  // Wait for card only if button was pressed
  if (waitingForCard && rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String cardID = getFormattedUID();
    Serial.println("Card detected: " + cardID);

    // Get current time
    struct tm timeinfo;
    time_t now;
    time(&now);

    // Find matching product
    for (const Product& product : products) {
      if (cardID == String(product.rfid)) {
        sendToAPI(product, digitalRead(IN_BUTTON) == LOW, now, 
                 digitalRead(OUT_BUTTON) == LOW ? now : 0);
        buzzerBeep();
        break;
      }
    }

    waitingForCard = false;
    buttonPressed = false;
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // Reset button state if released
  if (buttonPressed && digitalRead(IN_BUTTON) == HIGH && digitalRead(OUT_BUTTON) == HIGH) {
    buttonPressed = false;
  }

  delay(100);
}