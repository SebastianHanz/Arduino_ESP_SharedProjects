//Uploaded to GitHub 23.01.2021
#include <Arduino.h>

#include "BluetoothSerial.h"
#include <Adafruit_NeoPixel.h>
#define PIN 19
#define NUMPIXELS 60

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

String incomingMessage = "", led, red, green, blue;
int refreshLED;

void setLED(String Message)
{

  int einzelneVariablen[10];
  sscanf(Message.c_str(), "%d,%d,%d,%d,%d", &einzelneVariablen[0], &einzelneVariablen[1], &einzelneVariablen[2], &einzelneVariablen[3], &einzelneVariablen[4]);

  led = einzelneVariablen[0];
  red = einzelneVariablen[1];
  green = einzelneVariablen[2];
  blue = einzelneVariablen[3];
  refreshLED = einzelneVariablen[4];

  if (refreshLED == 1)
  {
    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
  }

  Serial.print("LED = ");
  Serial.println(led);
  Serial.print("red = ");
  Serial.println(red);
  Serial.print("green = ");
  Serial.println(green);
  Serial.print("blue = ");
  Serial.println(blue);
  Serial.print("Refresh = ");
  Serial.println(refreshLED);
  Serial.println();

  pixels.setPixelColor((led.toInt() - 1), pixels.Color(red.toInt(), green.toInt(), blue.toInt()));
  pixels.show();
}

void setup()
{
  pixels.begin();
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop()
{
  if (Serial.available())
  {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available())
  {
    //Serial.write(SerialBT.read());
    incomingMessage = SerialBT.readString();
    setLED(incomingMessage);
  }
  delay(20);
}
