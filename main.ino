#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_NeoPixel.h>


#define PIN 3
#define butt_pin 6
#define count_led 90
#define DHTPIN 2
#define PIRpin 0

DHT dht(DHTPIN, DHT11);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(count_led, PIN, NEO_GRB + NEO_KHZ800);

boolean butt_flag = 0;
boolean butt = 0;
boolean first_dht = 0;
unsigned long last_press;
unsigned long last_temp;
unsigned long last_pir;
unsigned long last_pir_diod;




void setup() {

  pinMode(butt_pin, INPUT_PULLUP);
  Serial.begin(9600);
  dht.begin();
  strip.begin();
  strip.setBrightness(255);
  strip.show(); // Initialize all pixels to 'off'

}
void loop() {

  if (millis() - last_pir_diod > 30000) {
    for (int i = 0; i < count_led; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0)); // Черный цвет, т.е. выключено.
    }
    strip.show();
  }


  if (analogRead(PIRpin) > 500 && millis() - last_pir > 10000)   {
    //Сигнал с датчика движения
    last_pir = millis();
    Serial.println("Есть движение!");
    for (int i = 0; i < count_led; i++) {
      strip.setPixelColor(i, strip.Color(255, 0, 0));
    }
    strip.show();
    last_pir_diod = millis();
  }
  
  dht_serial();
  rainbowCycle(2); // change for speed


}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  strip.setBrightness(255);
  butt = !digitalRead(butt_pin); // считать текущее положение кнопки
  if (butt == 1 && butt_flag == 0 && millis() - last_press > 500) {
    butt_flag = 1;
    last_press = millis();
    while (true) {
      for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
        dht_serial();
        butt = !digitalRead(butt_pin); // считать текущее положение кнопки
        if (butt == 1 && butt_flag == 1 && millis() - last_press > 500) {
          butt_flag = 0;
          for (int i = 0; i < count_led; i++) {
            strip.setPixelColor(i, strip.Color(0, 0, 0)); // Черный цвет, т.е. выключено.
          }
          // Передаем цвета ленте.
          strip.show();
          last_press = millis();
          return;
        } else {
          for (i = 0; i < strip.numPixels(); i++) {
            butt = !digitalRead(butt_pin); // считать текущее положение кнопки
            if (butt == 1 && butt_flag == 0 && millis() - last_press > 500) {
              butt_flag = 0;
              for (int i = 0; i < count_led; i++) {
                strip.setPixelColor(i, strip.Color(0, 0, 0)); // Черный цвет, т.е. выключено.
              }
              // Передаем цвета ленте.
              strip.show();
              last_press = millis();
              return;
            } else {
              strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
            }
          }
          strip.show();
          delay(wait);
        }
      }
      last_press = millis();
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void dht_serial() {
  if (Serial) {
    if (first_dht == 0) {
      int h = dht.readHumidity(); //Измеряем влажность
      int t = dht.readTemperature(); //Измеряем температуру
      first_dht = 1;
      if (isnan(h) || isnan(t)) {  // Проверка. Если не  считать показания, выводится «Ошибка считывания», и программа завершает работу
        Serial.println("Error");
        return;
      }
      Serial.print("Air humidity: ");
      Serial.print(h);
      Serial.print(" %\t");
      Serial.print("Air temp: ");
      Serial.print(t);
      Serial.println(" *C "); //Вывод показателей на экран
    }
    if (millis() - last_temp > 5000 && first_dht == 1) {
      int h = dht.readHumidity(); //Измеряем влажность
      int t = dht.readTemperature(); //Измеряем температуру
      last_temp = millis();
      if (isnan(h) || isnan(t)) {  // Проверка. Если не удается считать показания, выводится «Ошибка считывания», и программа завершает работу
        Serial.println("Error");
        return;
      }
      Serial.print("Air humidity: ");
      Serial.print(h);
      Serial.print(" %\t");
      Serial.print("Air temp: ");
      Serial.print(t);
      Serial.println(" *C "); //Вывод показателей на экран
    }
  }
}
