#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include <Wire.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_NeoPixel.h>
#include "GyverHacks.h"

#define LEDPIN 3
#define count_led 90
#define DHTPIN 2
#define PIRpin 0

DHT dht(DHTPIN, DHT11);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(count_led, LEDPIN, NEO_GRB + NEO_KHZ800);

boolean butt_flag;
boolean butt;
uint32_t last_press;
uint32_t last_temp;
uint32_t last_pir;


void dht_serial()
{
  if (millis() - last_temp > 5000)
  {
    int8_t h = dht.readHumidity();
    int8_t t = dht.readTemperature();
    last_temp = millis();
    if (isnan(h) || isnan(t))
    {
      Serial.println(F("Error"));
      return;
    }
    Serial.print(F("Air humidity: "));
    Serial.print(h);
    Serial.print(F(" %\t"));
    Serial.print(F("Air temp: "));
    Serial.print(t);
    Serial.println(F(" *C ")); //Вывод показателей на экран
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, c);
    strip.show();
    _delay_us(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85)
  {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else
  {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
// Slightly different, this makes the rainbow equally distributed throughout

void rainbowCycle(uint8_t wait)
{
  uint16_t i, j;
  if (!((PIND >> 6) & 1) && butt_flag == 0 && millis() - last_press > 500)
  {
    butt_flag = 1;
    last_press = millis();
    while (1)
    {
      for (j = 0; j < 256 * 5; j++)
      { // 5 cycles of all colors on wheel
        dht_serial();
        if (analogRead(PIRpin) > 500 && millis() - last_pir > 10000)
        {
          last_pir = millis();
          Serial.println(F("Есть движение!"));
        }
        if (!((PIND >> 6) & 1) && butt_flag == 1 && millis() - last_press > 500)
        {
          butt_flag = 0;
          for (int i = 0; i < count_led; i++)
          {
            strip.setPixelColor(i, strip.Color(0, 0, 0)); // Черный цвет, т.е. выключено.
          }
          strip.show();
          last_press = millis();
          return;
        }
        else
        {
          for (i = 0; i < strip.numPixels(); i++)
          {
            if (!((PIND >> 6) & 1) && butt_flag == 0 && millis() - last_press > 500)
            {
              butt_flag = 0;
              for (int i = 0; i < count_led; i++)
              {
                strip.setPixelColor(i, strip.Color(0, 0, 0)); // Черный цвет, т.е. выключено.
              }
              // Передаем цвета ленте.
              strip.show();
              last_press = millis();
              return;
            }
            else
            {
              strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
            }
          }
          strip.show();
          _delay_us(wait);
        }
      }
      last_press = millis();
    }
  }
}


int main()
{
  init();
  DDRD &=~(1<<6); PORTD |=1<<6;
  Serial.begin(9600);
  setADCrate(1);
  dht.begin();
  strip.begin();
  strip.show();

  for (;;)
  {
    _delay_us(3000);
    while (1)
    {
      if (analogRead(PIRpin) > 500 && millis() - last_pir > 10000)
      {
        last_pir = millis();
        Serial.println(F("Есть движение!"));
      }

      dht_serial();
      rainbowCycle(2); // change for speed
    }
  }
  return 0;
}
