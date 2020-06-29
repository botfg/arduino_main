#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include <Wire.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_NeoPixel.h>
#include <avdweb_AnalogReadFast.h>

#define LEDPIN 3
#define count_led 90
#define DHTPIN 5
#define PIRpin 0


DHT dht(DHTPIN, DHT11);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(count_led, LEDPIN, NEO_GRB + NEO_KHZ800);

volatile boolean butt_flag = 0;
volatile uint32_t timerPrew;
uint64_t last_temp;
uint64_t last_pir;

void dht_serial(void)
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

void rainbowCycle_button_2(void)
{
  uint16_t i, j;
  if (butt_flag == 1)
  {
    for (j = 0; j < 256 * 5; j++)
    { // 5 cycles of all colors on wheel
      for (i = 0; i < count_led; i++)
      {
        if (butt_flag == 0)
        {
          for (int i = 0; i < count_led; i++)
          {
            strip.setPixelColor(i, strip.Color(0, 0, 0)); // Черный цвет, т.е. выключено.
          }
          // Передаем цвета ленте.
          strip.show();
          return;
        }
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      }
      strip.show();
      _delay_us(2);
    }
  }
}


ISR(INT0_vect)
{
  if ((micros() - timerPrew) > 500000)
  {
    butt_flag = !butt_flag;
    timerPrew = micros();
  }
}

int main(void)
{
  init();
  _delay_us(2000);
  DDRD &= ~(1 << 2);
  PORTD |= 1 << 2; // button 6 pin input HIGH
  Serial.begin(9600);
  dht.begin();
  strip.begin();
  strip.show();

  //прерывания
  EICRA |= (1 << ISC10); // Устанавливаем ISC10 - отслеживаем  на INT0
  EIMSK |= (1 << INT0);  // Разрешаем прерывание INT0

  for (;;)
  {
    if (analogReadFast(PIRpin) > 500 && micros() - last_pir > 10000000)
    {
      last_pir = micros();
      Serial.println(F("Есть движение!"));
    }

    dht_serial();
    rainbowCycle_button_2();
  }
  return 0;
}