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

String receivedMessage = "", led, red, green, blue, Sunrise, Daylight, Sunset, Reset;

//General
int messageArray[10]; //Structure: [0]= FunctionCode; [1]=DataPacket1; [2]=DataPacket2...

//Sunrise/Sunset
int timerSunrise, timerDaylight, timerSunset, reset = 0;

//LED-Control
int refreshLED, RedLED, GreenLED, BlueLED, WhiteLED;
int LED_Counter = 0, LED_Counter_fade = 0, cntDelay;
uint8_t r, g, b;
uint32_t color;

byte Merker = 0;

unsigned long zeit_alt, proglaufzeit, LED_on_time = 1000; //(ms)

// Sonnenauf-/Untergang /////////////////////////////////////////////
void setTimer()
{
  timerSunrise = messageArray[1];  //[1]=DataPacket1;
  timerDaylight = messageArray[2]; //...
  timerSunset = messageArray[3];
  reset = messageArray[4]; //[4]=DataPacket4;
  Serial.println();
}

void fadeIn()
{
  for (int i = 0; i < NUMPIXELS * 5; i++)
  {
    LED_Counter_fade = LED_Counter - i;
    if (LED_Counter_fade <= NUMPIXELS)
    {
      color = pixels.getPixelColor(LED_Counter_fade);
      r = color >> 16;
      g = color >> 8;
      b = color;
      if (r < 205)
      {
        r = r + 50;
      }
      else
      {
        r = 255;
      }

      if (g < 254)
      {
        g = g + 1;
      }
      else
      {
        g = 255;
      }
      if (b < 254 && r > 190 && g > 55)
      {
        b = b + 1;
        /*} else {
          b = 255;*/
      }
      pixels.setPixelColor(LED_Counter_fade, pixels.Color(r, g, b));
    }
  }
  LED_Counter += 1;
  pixels.show();
}

void fadeOut()
{

  for (int i = 0; i > NUMPIXELS * (-4); i--)
  {
    LED_Counter_fade = LED_Counter - i;
    if (LED_Counter_fade >= NUMPIXELS * (-4))
    {
      color = pixels.getPixelColor(LED_Counter_fade);
      r = color >> 16;
      g = color >> 8;
      b = color;

      if (r >= 2)
      {
        r = r - 2;
      }

      else
      {
        r = 0;
      }

      if (g >= 4)
      {
        g = g - 4;
      }
      else
      {
        g = 0;
      }

      if (b >= 6)
      {
        b = b - 6;
      }
      else
      {
        b = 0;
      }
      pixels.setPixelColor(LED_Counter_fade, pixels.Color(r, g, b));
    }
  }
  LED_Counter = LED_Counter - 1;
  pixels.show();
}

void debugSunrise()
{
  Serial.print("receivedMessage= ");
  Serial.println(receivedMessage);
  Serial.print("reset= ");
  Serial.println(reset);
  Serial.print("Led-ontime ");
  Serial.println(LED_on_time);
  Serial.println(LED_Counter);
  Serial.println(Merker);
  Serial.println(NUMPIXELS * 5);
  Serial.print("Sunrise=");
  Serial.println(timerSunrise);
  Serial.print("Daylight=");
  Serial.println(timerDaylight);
  Serial.print("Sunset=");
  Serial.println(timerSunset);
  Serial.print("cntDelay=");
  Serial.println(cntDelay);
  Serial.print("LED_FADE=");
  Serial.println(LED_Counter_fade);
}
// ENDE Sonnenauf-/Untergang /////////////////////////////////////////////

// LED-Control  /////////////////////////////////////////////////////////
void setLED()
{

  led = messageArray[1];
  red = messageArray[2];
  green = messageArray[3];
  blue = messageArray[4];
  refreshLED = messageArray[5];
  RedLED = messageArray[6];
  GreenLED = messageArray[7];
  BlueLED = messageArray[8];
  WhiteLED = messageArray[9];

  if (refreshLED == 1)
  {
    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
  }
  if (RedLED == 1)
  {
    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0));
    }
  }
  if (GreenLED == 1)
  {
    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 255, 0));
    }
  }
  if (BlueLED == 1)
  {
    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 255));
    }
  }

  if (WhiteLED == 1)
  {
    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(255, 255, 255));
    }
  }
  Serial.print("receivedMessage= ");
  Serial.println(receivedMessage);
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
  Serial.print("RedLED = ");
  Serial.println(RedLED);
  Serial.print("GreenLED = ");
  Serial.println(GreenLED);
  Serial.print("BlueLED = ");
  Serial.println(BlueLED);
  Serial.print("WhiteLED = ");
  Serial.println(WhiteLED);
  Serial.println();

  pixels.setPixelColor((led.toInt() - 1), pixels.Color(red.toInt(), green.toInt(), blue.toInt()));
  pixels.show();
}

// ENDE LED-Control  /////////////////////////////////////////////////////
void setup()
{
  zeit_alt = millis();
  pixels.begin();
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop()
{
  //Read serial datastream from BTSerial into receivedMessage
  if (SerialBT.available())
  {
    receivedMessage = SerialBT.readString();

    //Seperate message-string into 'DataPackets' seperated by ","
    sscanf(receivedMessage.c_str(), "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &messageArray[0], &messageArray[1], &messageArray[2], &messageArray[3], &messageArray[4], &messageArray[5], &messageArray[6], &messageArray[7], &messageArray[8], &messageArray[9]);
  }
  //delay(20);

  if (receivedMessage != "")
  {

    //FunctionCode -> Sunrise/Sunset
    if (messageArray[0] == 1)
    {
      setTimer();

      debugSunrise(); //Prints used data to serial

      if ((millis() - zeit_alt) >= LED_on_time)
      {
        zeit_alt = millis();

        if (((LED_Counter <= NUMPIXELS * 5) && (Merker == 0) && (reset == 0) || ((LED_Counter <= NUMPIXELS * (-4)) && (Merker == 3))))
        {
          Merker = 1;
          LED_Counter_fade = 0;
          LED_Counter = 0;
          LED_on_time = (timerSunrise * (200)); // LED_on_time=(timerSunrise*200); (=) wenn für timerSunrise=1 eingegeben wird entspricht 1 Minute Hochlaufzeit
        }

        else if ((LED_Counter >= NUMPIXELS * 5) && (Merker == 1))
        {
          Merker = 2;
          LED_on_time = (timerDaylight * (200));
        }

        else if (Merker == 2 && (cntDelay <= LED_on_time) && (cntDelay <= 0))
        { // timeWait=5 (Sekunden) -> (5 * (1000/1))/4 = 5 Sekunden    1 war hier LED_on_time und wurde von mir gesetzt
          Merker = 3;
          LED_Counter = 60;
          LED_on_time = (timerSunset * (200));
        }
        else if ((LED_Counter == 0) && (Merker == 3 && Merker == 2) && (cntDelay >= (NUMPIXELS * 5)) || (reset == 1))
        {
          Merker = 0;
        }

        if (Merker == 1)
        {
          Serial.println("In FadeIn-Schleife");
          cntDelay = (NUMPIXELS * 5);
          fadeIn();
        }
        else if (Merker == 2)
        {
          Serial.println("In Warte-Schleife");
          cntDelay--;
        }
        else if (Merker == 3)
        {
          Serial.println("In FadeOut-Schleife");
          cntDelay++;
          fadeOut();
        }
        else if (Merker == 0)
        {
          Serial.println("ResetSchleife");
          LED_Counter_fade = 0;
          LED_Counter = 0;
          LED_on_time = 0;
          cntDelay = 0;
          for (int i = 0; i < NUMPIXELS; i++)
          {
            pixels.setPixelColor(i, pixels.Color(0, 0, 0));
            pixels.show();
          }
        }
        else
        {
          Serial.println("Merker ELSE");
        }
      }
    }

    //FunctionCode -> LED-Control
    else if (messageArray[0] == 2)
    {
      setLED();
      Merker = 0;
    }
  }
}
