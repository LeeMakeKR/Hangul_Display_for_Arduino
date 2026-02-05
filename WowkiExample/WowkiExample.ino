/*
  ESP32 HTTPClient Jokes API Example

  https://wokwi.com/projects/342032431249883731

  Copyright (C) 2022, Uri Shaked
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "hangulDisp.h"
#include "H01_kr.h"

#define TFT_DC 2
#define TFT_CS 15
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void drawPixel(int16_t x, int16_t y, uint16_t color) {
  tft.drawPixel(x, y, color);
}

hangulDisp hangul(drawPixel);

void setup() {
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  hangul.setFont(H01_kr);
  hangul.print(10, 30, "안녕하세요 한글출력입니다", ILI9341_WHITE);
}

void loop() {}