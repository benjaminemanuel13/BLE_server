/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "6566B82D-7C80-447D-B221-FF55761F8401"
#define CHARACTERISTIC_UUID "C7228C2F-429C-47D7-9943-52C36FB7EF85"

int horzPin = 12; // Analog output of horizontal joystick pin
int vertPin = 13;
int swPin = 14;
int lastDebounceTime=0;

int vertZero, horzZero, swZero; // Stores the initial value of each axis, usually around 512
int vertValue, horzValue, swValue; // Stores current analog output of each axis

const int sensitivity = 200; // Higher sensitivity value = slower mouse, should be <= about 500

BLECharacteristic *pCharacteristic;
//BLECharacteristic pCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
BLEService *pService;
BLEServer *pServer;

signed char limit_xy(int const xy)
{
  if (xy < -127) return -127;

  else if(xy > 127) return 127;

  else return xy;
}

bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("Bens AR Control Device");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  pService = pServer->createService(SERVICE_UUID);
  
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ
                                         | BLECharacteristic::PROPERTY_WRITE
                                         | BLECharacteristic::PROPERTY_NOTIFY
                                       );

  pCharacteristic->setValue("Hello World");

  BLEDescriptor desc(BLEUUID((uint16_t)0x2902));
  desc.setValue("Joystick");
  pCharacteristic->addDescriptor(&desc);

  pService->addCharacteristic(pCharacteristic);
  
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  pinMode(horzPin, INPUT); // Set both analog pins as inputs
  pinMode(vertPin, INPUT);
  pinMode(swPin, INPUT);
  
  delay(1000); // short delay to let outputs settle

  vertZero = analogRead(vertPin); // get the initial values
  horzZero = analogRead(horzPin); // Joystick should be in neutral position when reading these
  swZero = analogRead(swPin);
}

void loop() {
  // put your main code here, to run repeatedly:
  if ((millis() - lastDebounceTime) > 20) {

    lastDebounceTime = millis();
    vertValue = analogRead(vertPin)- vertZero ;
    horzValue = analogRead(horzPin)- horzZero ;
    swValue = analogRead(swPin)- swZero ;

    //Serial.println(swValue);
    //return;
    if (vertValue != 0)
    {
      if(vertValue < -500)
      {
        Serial.println("Up");
      }
      else if(vertValue > 500)
      {
        Serial.println("Down");
      }
    }
  
    if (horzValue != 0)
    {
      if(horzValue < -500)
      {
        Serial.println("Right");
        pCharacteristic->setValue("LeftRight");
        pCharacteristic->notify();
      }
      else if(horzValue > 500)
      {
        Serial.println("Left");
        pCharacteristic->setValue("LeftLeft");
        pCharacteristic->notify();
      }
    }

    if(swValue < -800)
    {
      Serial.println("Fire");
    }
  }

  delay(20);
}
