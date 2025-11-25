# RFID Based Inventory Management System

This project is an IoT-based inventory management system that uses RFID technology to track the movement of products. It allows for real-time check-in and check-out of items, updating a central database via a REST API.

## Features

*   **Real-time Tracking**: Instantly updates inventory status when items are scanned.
*   **Check-in & Check-out**: Dedicated buttons to toggle between adding stock (Check-in) and removing stock (Check-out).
*   **Cloud Integration**: Syncs data with a Convex backend for persistent storage and management.
*   **Audio Feedback**: Buzzer provides audible confirmation for successful operations and error alerts.
*   **Automated Timestamping**: Records entry and exit times automatically.

## Hardware Requirements

To build this system, you will need the following hardware components:

*   **Microcontroller**: ESP32 Development Board
*   **RFID Reader**: MFRC522 Module
*   **RFID Tags/Cards**: Compatible 13.56MHz tags
*   **Push Buttons**: 2x (One for IN, One for OUT)
*   **Buzzer**: 1x (Active or Passive)
*   **Jumper Wires**: Male-to-Male and Male-to-Female
*   **Breadboard**: For prototyping

### Pin Connections

| Component | Pin Name | ESP32 Pin |
| :--- | :--- | :--- |
| **MFRC522** | SDA (SS) | GPIO 5 |
| | SCK | GPIO 18 |
| | MOSI | GPIO 23 |
| | MISO | GPIO 19 |
| | RST | GPIO 22 |
| | GND | GND |
| | 3.3V | 3.3V |
| **Buttons** | IN Button | GPIO 25 |
| | OUT Button | GPIO 26 |
| **Buzzer** | Positive (+) | GPIO 27 |

*Note: Buttons should be connected between the GPIO pin and GND (using internal pull-ups).*

## Software Requirements

*   **Arduino IDE**: Download and install the latest version.
*   **ESP32 Board Support**: Install the ESP32 board manager in Arduino IDE.

### Required Libraries

Install the following libraries via the Arduino Library Manager:

1.  **MFRC522** by GithubCommunity (for RFID communication)
2.  **WiFi** (Built-in for ESP32)
3.  **HTTPClient** (Built-in for ESP32)
4.  **SPI** (Built-in)

## Installation & Setup

1.  **Clone the Repository**:
    ```bash
    git clone <repository-url>
    ```
2.  **Open in Arduino IDE**: Open the `RFID_based_inventory_management.ino` file.
3.  **Configure WiFi**:
    *   Locate the `ssid` and `password` variables in the code.
    *   Replace them with your WiFi network credentials.
    ```cpp
    const char* ssid = "YOUR_WIFI_SSID";
    const char* password = "YOUR_WIFI_PASSWORD";
    ```
4.  **Configure API Endpoints**:
    *   Ensure the `insertServerName` and `updateServerName` variables point to your valid Convex backend URLs.
5.  **Select Board**: Go to `Tools` > `Board` and select your specific ESP32 board (e.g., "DOIT ESP32 DEVKIT V1").
6.  **Upload**: Connect your ESP32 via USB and click the Upload button.

## Usage

1.  **Power On**: Connect the ESP32 to a power source. Open the Serial Monitor (115200 baud) to view status logs.
2.  **Wait for Connection**: The system will attempt to connect to WiFi. Once connected, it will print "System ready!".
3.  **Check-In (Add Stock)**:
    *   Press the **IN Button** (GPIO 25).
    *   Scan an RFID tag.
    *   **Success**: Buzzer beeps once. Inventory count increases.
    *   **Error**: Buzzer beeps twice (e.g., WiFi error).
4.  **Check-Out (Remove Stock)**:
    *   Press the **OUT Button** (GPIO 26).
    *   Scan an RFID tag.
    *   **Success**: Buzzer beeps once. Inventory count decreases.
    *   **Error**: Buzzer beeps twice (e.g., Item not found, Out of stock).

## API Integration

The system communicates with a backend using JSON payloads.

*   **POST /insertItem**: Used when a new product is scanned for the first time (Check-in).
*   **PUT /updateItem**: Used to update the quantity and timestamps of existing products.

## Troubleshooting

*   **WiFi Not Connecting**: Check your SSID and Password. Ensure the network is 2.4GHz (ESP32 does not support 5GHz).
*   **RFID Not Reading**: Check wiring connections. Ensure the tag is compatible (Mifare 1k/4k).
*   **API Errors**: Verify the server URLs are reachable and the backend is running.
