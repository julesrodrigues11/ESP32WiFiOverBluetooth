#include <WiFi.h>              // WiFi library
#include "BluetoothSerial.h"    // Bluetooth Serial library

BluetoothSerial SerialBT;       // Create BluetoothSerial instance
const int ledPin = 2;           // Define onboard LED pin (usually GPIO 2)

void setup() {
  Serial.begin(115200);         // Initialize serial monitor for debugging
  SerialBT.begin("ESP32_BT");   // Initialize Bluetooth with device name "ESP32_BT"
  pinMode(ledPin, OUTPUT);      // Set LED pin as output
  digitalWrite(ledPin, LOW);    // Turn off the LED initially
  
  Serial.println("Waiting for Bluetooth connection to receive WiFi credentials...");
}

void loop() {
  if (SerialBT.available()) {                 // Check if Bluetooth data is available
    String wifiData = SerialBT.readString();  // Read Bluetooth data (WiFi SSID and password)
    Serial.print("Received WiFi credentials: ");
    Serial.println(wifiData);

    // Split the incoming data by the comma to extract SSID and password
    int commaIndex = wifiData.indexOf(',');
    if (commaIndex != -1) {
      String ssid = wifiData.substring(0, commaIndex);
      ssid.trim();
      String password = wifiData.substring(commaIndex + 1);
      password.trim();
      
      Serial.println("Disconnecting Bluetooth...");
      SerialBT.end();    // Disconnect Bluetooth after receiving WiFi credentials
      delay(1000);       // Give time for Bluetooth to shut down

      Serial.println("Connecting to WiFi...");
      WiFi.begin(ssid.c_str(), password.c_str());   // Begin WiFi connection
      
      // Wait for connection
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        digitalWrite(ledPin, HIGH);   // Turn on LED to indicate successful connection
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());  // Print IP address
      } else {
        Serial.println("\nFailed to connect to WiFi");
        digitalWrite(ledPin, LOW);    // Ensure LED is off if connection fails
      }
    } else {
      Serial.println("Invalid format. Send as SSID,PASSWORD");
    }
  }
  
  delay(100);   // Small delay to avoid overloading the loop
}