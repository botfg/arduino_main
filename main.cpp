#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include <Wire.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SFE_BMP180.h>
#include <Adafruit_NeoPixel.h>
#include <avdweb_AnalogReadFast.h>

#define LEDPIN 3
#define count_led 90
#define DHTPIN 5
#define PIRpin 0

SFE_BMP180 pressure;

DHT dht(DHTPIN, DHT11);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(count_led, LEDPIN, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(16, 8, NEO_GRB + NEO_KHZ800);

volatile boolean butt_flag = 0;
volatile uint64_t timerPrew;
uint64_t last_temp;
uint64_t last_pir;
uint64_t last_bmp;

void bmp180(void)
{
  if (micros() - last_bmp > 10000000)
  {
    char status;
    double T, P;
    int8_t h = dht.readHumidity();


    status = pressure.startTemperature();
    if (status != 0)
    {
      // ждем:
      delay(status);
      status = pressure.getTemperature(T);
      if (status != 0)
      {
        Serial.println();
        Serial.println();
        Serial.print(F("Температура: "));
        Serial.print(T, 2);
        Serial.print(F(" градусов C, "));
        Serial.print(F(" \t"));
        Serial.print(F("Air humidity: "));
        Serial.println(h);
        Serial.println();
      }
    }

    status = pressure.startPressure(3);
    if (status != 0)
    {
      delay(status);
      // Теперь можно получить давление в переменную P.
      //Функция вернет 1 если все ОК, 0 если не ОК.
      status = pressure.getPressure(P, T);
      if (status != 0)
      {
        Serial.print(F("Абсолютное давление: "));
        Serial.print(P, 2);
        Serial.print(F(" миллибар, "));
        Serial.print(P * 0.750064, 2);
        Serial.println(F(" мм ртутного столба"));
      }
    }
    last_bmp = micros();
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
  if ((micros() - timerPrew) > 200000)
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
  pressure.begin();
  strip.begin();
  strip.show();
  strip2.begin();
  strip2.show();

  for (int i = 0; i < 16; i++)
  {
    strip2.setPixelColor(i, strip2.Color(0, 20, 0));
  }

  strip2.show();

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

    rainbowCycle_button_2();
    bmp180();
  }
  return 0;
}