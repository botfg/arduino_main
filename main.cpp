#include <Arduino.h>
#include <Wire.h>
#include <SFE_BMP180.h> 
#include <SPI.h>                  
#include <Adafruit_NeoPixel.h>





#define LEDPIN 6
#define count_led 90

#define DHT_ERROR_CHECKSUM -1 // Ошибка контрольной суммы
#define DHT_ERROR_TIMEOUT -2  // Ошибка таймаут



SFE_BMP180 pressure;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(count_led, LEDPIN, NEO_GRB + NEO_KHZ800);



volatile boolean butt_flag;
volatile uint64_t timerPrew;
uint64_t last_bmp;

volatile const int data = 1;

int16_t DHT11(void)
{
  byte data[16];

  DDRD |= 1 << 5;
  PORTD &= ~(1 << 5);
  delay(18);

  PORTD |= 1 << 5;
  delayMicroseconds(40);
  DDRD &= ~(1 << 5);

  uint32_t timeoutInt = 10000; // Создаем переменную, отслеживающую таймаут

  while (((PIND >> 5) & 1) == LOW)
    if (timeoutInt-- == 0)
      return DHT_ERROR_TIMEOUT;

  timeoutInt = 10000;
  while (((PIND >> 5) & 1) == HIGH)
    if (timeoutInt-- == 0)
      return DHT_ERROR_TIMEOUT;

  // Если дошли до этого места, значит все нормально - начинаем принимать данные
  // Нам нужно принять 16 бит
  for (byte i = 0; i < 16; i++)
  {
    timeoutInt = 10000;
    while (((PIND >> 5) & 1) == LOW)
      if (timeoutInt-- == 0)
        return DHT_ERROR_TIMEOUT;

    unsigned long t = micros();
    timeoutInt = 10000;
    while (((PIND >> 5) & 1) == HIGH)
      if (timeoutInt-- == 0)
        return DHT_ERROR_TIMEOUT;

    // По интервалу определяем, какой бит был передан
    // Если больше 40 (нам нужно 70 мкс) - значит 1, если меньше 40 (27 мкс) - значит 0
    if ((micros() - t) > 40)
      data[i] = 1;
    else
      data[i] = 0; // Инициализация нулевого значения
  }

  // Собираем байты из битов
  byte bytes[5] = {0, 0}; // Инициализируем байты для хранения полученных данных
  int16_t byteindex = 0;      // Индекс байта
  int16_t posindex = 7;       // Индекс бита в байте. Данные приходят в порядке: первым приходит старший бит
  for (byte i = 0; i < 16; i++)
  {
    if (data[i] == 1)
      bytes[byteindex] |= (1 << posindex); // Задаем бит при помощи побитового ИЛИ
    posindex--;
    if (posindex < 0)
    {
      posindex = 7;
      byteindex++; // Переходим к следующему байту
    }
  }

  // Присваиваем полученные данные переменным температуры и влажности
  byte humidity = bytes[0];

  return humidity;
}

void bmp180(void)
{
  if (micros() - last_bmp > 30000000)
  {
    char status;
    double T, P;


    status = pressure.startTemperature();
    if (status != 0)
    {
      // ждем:
      delay(status);
      status = pressure.getTemperature(T);
      int16_t result = (*DHT11)();
      if (status != 0)
      {
        Serial.println();
        Serial.println();
        Serial.print(F("Температура: "));
        Serial.print(T, 2);
        Serial.print(F(" °C"));
        Serial.print(F(" Влажность: "));
        Serial.print(result);
        Serial.print(F(" %"));
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
  if (butt_flag == 1)
  {
    for (byte j = 0; j < 256 * 5; j++)
    { // 5 cycles of all colors on wheel
      for (byte i = 0; i < count_led; i++)
      {
        if (butt_flag == 0)
        {
          for (byte i = 0; i < count_led; i++)
          {
            strip.setPixelColor(i, strip.Color(0, 0, 0)); // Черный цвет, т.е. выключено.
          }
          // Передаем цвета ленте.
          strip.show();
          return;
        }
        strip.setPixelColor(i, (*Wheel)(((i * 256 / strip.numPixels()) - j) & 255));
      }
      strip.show();
      delay(2);
    }
  }
}


ISR(INT0_vect)
{
  if ((micros() - timerPrew) > 50000)
  {
    butt_flag = !butt_flag;
    timerPrew = micros();
  }
}




int main()
{
  init();
  cli();
  delay(2000);
  DDRD &= ~(1 << 2);
  PORTD |= 1 << 2; // button 6 pin input HIGH
  //прерывания внешние
  EICRA |= (1 << ISC10); // Устанавливаем ISC10 - отслеживаем  на INT0
  EIMSK |= (1 << INT0);  //   прерывание INT0
  sei();                 //разрешить прерывания
  Serial.begin(9600);
  pressure.begin();
  strip.begin();
  strip.show();



  for (;;)
  {
    rainbowCycle_button_2();
    bmp180();
  }
  return 0;
}
