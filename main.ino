#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

// pins
#define SDA_PIN 9
#define SCL_PIN 10
#define BTN_POWER 4
#define BTN_MODE  5
#define LED_PIN   15

// strip + oled
#define NUM_LEDS 30
#define OLED_ADDR 0x3C
#define W 128
#define H 64

Adafruit_SSD1306 oled1(W, H, &Wire, -1);
Adafruit_SSD1306 oled2(W, H, &Wire, -1);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// state
bool on = false;
bool progressive = false;

// teal
uint32_t teal;

// for progressive animation
int prog = 0;
unsigned long lastStep = 0;
const int stepDelayMs = 60;

// simple button memory
bool lastPower = HIGH;
bool lastMode  = HIGH;
unsigned long lastPressPower = 0;
unsigned long lastPressMode  = 0;
const int debounceMs = 30;

void showText(const char *txt) {
  // left
  oled1.clearDisplay();
  oled1.setTextSize(4);
  oled1.setTextColor(SSD1306_WHITE);
  oled1.setCursor(20, 20);
  oled1.print(txt);
  oled1.display();

  // right
  oled2.clearDisplay();
  oled2.setTextSize(4);
  oled2.setTextColor(SSD1306_WHITE);
  oled2.setCursor(20, 20);
  oled2.print(txt);
  oled2.display();
}

void setStripOff() {
  strip.clear();
  strip.show();
}

void setStripStatic() {
  for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, teal);
  strip.show();
}

void doProgressiveStep() {
  strip.clear();
  for (int i = 0; i <= prog; i++) strip.setPixelColor(i, teal);
  strip.show();

  prog++;
  if (prog >= NUM_LEDS) prog = 0;
}

void updateDisplayAndLeds() {
  prog = 0;
  lastStep = millis();

  if (!on) {
    setStripOff();
    showText("OFF");
    return;
  }

  if (!progressive) {
    setStripStatic();
    showText("S");
  } else {
    showText("P");
  }
}

void setup() {
  pinMode(BTN_POWER, INPUT_PULLUP);
  pinMode(BTN_MODE,  INPUT_PULLUP);

  Wire.begin(SDA_PIN, SCL_PIN);

  oled1.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  oled2.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  showText("OFF");

  strip.begin();
  strip.setBrightness(80);
  teal = strip.Color(0, 128, 128);
  setStripOff();
}

void loop() {
  bool p = digitalRead(BTN_POWER);
  bool m = digitalRead(BTN_MODE);

  // power button 
  if (p == LOW && lastPower == HIGH && (millis() - lastPressPower) > debounceMs) { //simple 30ms debounce
    lastPressPower = millis();
    on = !on;
    updateDisplayAndLeds();
  }
  lastPower = p;

  // mode button, toggle static/progressive
  if (m == LOW && lastMode == HIGH && (millis() - lastPressMode) > debounceMs) { //simple 30ms debounce
    lastPressMode = millis();
    progressive = !progressive;
    updateDisplayAndLeds();
  }
  lastMode = m;

  // run progressive animation 
  if (on && progressive) {
    if (millis() - lastStep > stepDelayMs) {
      lastStep = millis();
      doProgressiveStep();
    }
  }
}
