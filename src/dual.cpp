/*
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BluetoothSerial.h>
#include <WiFi.h>

// Check if Bluetooth configs are enabled
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Bluetooth Serial object
BluetoothSerial SerialBT;

// BLE Server
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

const int ledPin = 2;  // Onboard LED pin
String wifiCredentials = "";

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void connectToWiFi(String credentials);

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        wifiCredentials = String(value.c_str());
        Serial.println("Received WiFi credentials via BLE: " + wifiCredentials);
        connectToWiFi(wifiCredentials);
      }
    }
};

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Initialize Classic Bluetooth
  SerialBT.begin("ESP32_BT");
  Serial.println("Classic Bluetooth started. Device name: ESP32_BT");

  // Initialize BLE
  BLEDevice::init("ESP32_BLE");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();
  Serial.println("BLE started. Device name: ESP32_BLE");

  Serial.println("Waiting for Bluetooth connection (Classic or BLE)...");
}

void connectToWiFi(String credentials) {
  int commaIndex = credentials.indexOf(',');
  if (commaIndex != -1) {
    String ssid = credentials.substring(0, commaIndex);
    String password = credentials.substring(commaIndex + 1);
    ssid.trim();
    password.trim();
    
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
      digitalWrite(ledPin, HIGH);
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      
      String successMessage = "WiFi Connected. IP: " + WiFi.localIP().toString();
      SerialBT.println(successMessage);
      if (deviceConnected) {
        pCharacteristic->setValue(successMessage.c_str());
        pCharacteristic->notify();
      }
    } else {
      Serial.println("\nFailed to connect to WiFi");
      digitalWrite(ledPin, LOW);
      
      String failureMessage = "WiFi Connection Failed";
      SerialBT.println(failureMessage);
      if (deviceConnected) {
        pCharacteristic->setValue(failureMessage.c_str());
        pCharacteristic->notify();
      }
    }
  } else {
    Serial.println("Invalid format. Send as SSID,PASSWORD");
    SerialBT.println("Invalid format. Send as SSID,PASSWORD");
    if (deviceConnected) {
      pCharacteristic->setValue("Invalid format. Send as SSID,PASSWORD");
      pCharacteristic->notify();
    }
  }
}

void loop() {
  // Handle Classic Bluetooth
  if (SerialBT.available()) {
    wifiCredentials = SerialBT.readString();
    Serial.println("Received WiFi credentials via Classic Bluetooth: " + wifiCredentials);
    connectToWiFi(wifiCredentials);
  }

  // Handle BLE connection status changes
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("BLE: Start advertising");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  delay(20);
}
*/