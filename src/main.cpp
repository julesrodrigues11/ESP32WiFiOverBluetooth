#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
String wifiCredentials = "";

#define SERIAL_DEBUG_BAUD 115200

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

const int ledPin = 2;  // Define onboard LED pin - For debugging purposes only

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        wifiCredentials = String(value.c_str());
        Serial.println("Received WiFi credentials: " + wifiCredentials);
      }
    }
};

void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // Turn off the LED initially

  // Create the BLE Device
  BLEDevice::init("ESP32_BLE");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection to notify...");
}

void loop() {
  // Handle WiFi connection if credentials are received
  if (wifiCredentials.length() > 0) {
    int commaIndex = wifiCredentials.indexOf(',');
    if (commaIndex != -1) {
      String ssid = wifiCredentials.substring(0, commaIndex);
      String password = wifiCredentials.substring(commaIndex + 1);
      
      Serial.println("Connecting to WiFi...");
      WiFi.begin(ssid.c_str(), password.c_str());
      
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        digitalWrite(ledPin, HIGH);  // Turn on LED to indicate successful connection
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        
        // Notify the client about successful connection
        if (deviceConnected) {
          pCharacteristic->setValue("WiFi Connected");
          pCharacteristic->notify();
        }
      } else {
        Serial.println("\nFailed to connect to WiFi");
        digitalWrite(ledPin, LOW);  // Ensure LED is off if connection fails
        
        // Notify the client about connection failure
        if (deviceConnected) {
          pCharacteristic->setValue("WiFi Connection Failed");
          pCharacteristic->notify();
        }
      }
      
      wifiCredentials = "";  // Reset credentials after attempt
    }
  }
  
  // Disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // Restart advertising
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // Connecting
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
  
  delay(1000);
}
