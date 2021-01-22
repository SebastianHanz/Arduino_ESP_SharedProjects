//Uploaded on GitHub 22.01.2021
#include <Adafruit_NeoPixel.h>
#define PIN         19
#define NUMPIXELS   60

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Arduino.h>

String valor, led, red, green, blue;
int refreshLED;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        valor = "";
        for (int i = 0; i < value.length(); i++) {
          valor = valor + value[i];
        }

        int einzelneVariablen[10];
        sscanf(valor.c_str(), "%d,%d,%d,%d,%d", &einzelneVariablen[0], &einzelneVariablen[1], &einzelneVariablen[2], &einzelneVariablen[3], &einzelneVariablen[4]);

        led = einzelneVariablen[0];
        red = einzelneVariablen[1];
        green = einzelneVariablen[2];
        blue = einzelneVariablen[3];
        refreshLED = einzelneVariablen[4];

        if (refreshLED == 1) {
          for (int i = 0; i < NUMPIXELS; i++) {
            pixels.Color(i, 0, 0, 0);
          }
        }

        Serial.print("LED = ");Serial.println(led);
        Serial.print("red = ");Serial.println(red);
        Serial.print("green = ");Serial.println(green);
        Serial.print("blue = ");Serial.println(blue);
        Serial.print("Refresh = ");Serial.println(refreshlED);
        Serial.println();
        
        pixels.setPixelColor((led.toInt() - 1), pixels.Color(red.toInt(), green.toInt(), blue.toInt()));
        pixels.show();

        pCharacteristic->setValue(valor.c_str()); 
      }
    }
};

void setup() {
  pixels.begin();
  Serial.begin(115200);

  BLEDevice::init("MyESP32");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Iniciado.");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  //
}
