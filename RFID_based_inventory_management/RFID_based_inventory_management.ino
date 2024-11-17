
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

// API endpoints
const char* insertServerName = "https://deafening-gnu-222.convex.site/insertItem";
const char* updateServerName = "https://deafening-gnu-222.convex.site/updateItem";

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

// Product status structure with timestamps
struct ProductStatus {
  String rfid;
  bool isInitialized;
  int currentQuantity;
  time_t lastInTime;
  time_t lastOutTime;
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

// Initialize product status array
ProductStatus productStatuses[sizeof(products) / sizeof(products[0])];

bool buttonPressed = false;
bool waitingForCard = false;

// Function to get formatted RFID UID
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

// Buzzer feedback functions
void buzzerSuccess() {
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
}

void buzzerError() {
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
  delay(100);
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
}

// Product status management functions
bool isProductInitialized(const String& rfid) {
  for (const ProductStatus& status : productStatuses) {
    if (status.rfid == rfid) {
      return status.isInitialized;
    }
  }
  return false;
}

int getCurrentQuantity(const String& rfid) {
  for (const ProductStatus& status : productStatuses) {
    if (status.rfid == rfid) {
      return status.currentQuantity;
    }
  }
  return 0;
}

// Find product by RFID
const Product* findProductByRFID(const String& rfid) {
  for (const Product& product : products) {
    if (String(product.rfid) == rfid) {
      return &product;
    }
  }
  return nullptr;
}

// Handle API communication
void sendToAPI(const Product& product, bool isIn, time_t timestamp, time_t unused) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    buzzerError();
    return;
  }

  HTTPClient http;
  String rfidStr = String(product.rfid);
  bool productExists = isProductInitialized(rfidStr);
  int currentQty = getCurrentQuantity(rfidStr);
  
  // Get previous timestamps
  time_t previousInTime = 0;
  time_t previousOutTime = 0;
  for (const ProductStatus& status : productStatuses) {
    if (status.rfid == rfidStr) {
      previousInTime = status.lastInTime;
      previousOutTime = status.lastOutTime;
      break;
    }
  }
  
  // Error handling for invalid operations
  if (!isIn && !productExists) {
    Serial.println("Error: Cannot check out product that was never checked in!");
    buzzerError();
    return;
  }
  
  if (!isIn && currentQty <= 0) {
    Serial.println("Error: Cannot check out product - quantity is zero!");
    buzzerError();
    return;
  }

  String jsonPayload;
  const char* endpoint;
  
  if (!productExists && isIn) {
    // First time insertion
    endpoint = insertServerName;
    jsonPayload = "{\"name\":\"" + String(product.name) + 
                  "\",\"categoryId\":\"" + String(product.categoryId) + 
                  "\",\"quantity\":" + String(product.quantity) + 
                  ",\"price\":" + String(product.price) + 
                  ",\"rfid\":\"" + rfidStr + 
                  "\",\"intime\":" + String(timestamp) + 
                  ",\"outtime\":0" + 
                  ",\"isIn\":true}";
  } else {
    // Update existing product
    endpoint = updateServerName;
    int newQuantity = isIn ? currentQty + 1 : currentQty - 1;
    
    jsonPayload = "{\"name\":\"" + String(product.name) + 
                  "\",\"categoryId\":\"" + String(product.categoryId) + 
                  "\",\"price\":" + String(product.price) + 
                  ",\"rfid\":\"" + rfidStr + 
                  "\",\"quantity\":" + String(newQuantity);

    if (isIn) {
        jsonPayload += String(",\"intime\":") + String(timestamp) + 
                      String(",\"outtime\":") + String(previousOutTime);
    } else {
        jsonPayload += String(",\"intime\":") + String(previousInTime) + 
                      String(",\"outtime\":") + String(timestamp);
    }

    jsonPayload += String(",\"isIn\":") + (isIn ? "true" : "false") + "}";
  }
  
  Serial.println("Sending " + String(productExists ? "PUT" : "POST") + " request with payload: " + jsonPayload);
  
  http.begin(endpoint);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = productExists ? http.PUT(jsonPayload) : http.POST(jsonPayload);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Response code: " + String(httpResponseCode));
    Serial.println("Response: " + response);
    
    // Update product status including timestamps
    for (ProductStatus& status : productStatuses) {
      if (status.rfid == rfidStr) {
        status.isInitialized = true;
        status.currentQuantity = isIn ? (productExists ? currentQty + 1 : product.quantity) : currentQty - 1;
        if (isIn) {
          status.lastInTime = timestamp;
        } else {
          status.lastOutTime = timestamp;
        }
        break;
      }
    }
    
    buzzerSuccess();
  } else {
    Serial.println("Error on sending " + String(productExists ? "PUT" : "POST") + " Request");
    buzzerError();
  }
  
  http.end();
}

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(IN_BUTTON, INPUT_PULLUP);
  pinMode(OUT_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  
  // Initialize RFID
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  rfid.PCD_Init();

  // Initialize product statuses with timestamps
  for (size_t i = 0; i < sizeof(products) / sizeof(products[0]); i++) {
    productStatuses[i].rfid = String(products[i].rfid);
    productStatuses[i].isInitialized = false;
    productStatuses[i].currentQuantity = 0;
    productStatuses[i].lastInTime = 0;
    productStatuses[i].lastOutTime = 0;
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Initialize time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  Serial.println("System ready!");
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
    const Product* product = findProductByRFID(cardID);
    if (product != nullptr) {
      sendToAPI(*product, digitalRead(IN_BUTTON) == LOW, now, 0);
    } else {
      Serial.println("Unknown RFID card!");
      buzzerError();
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