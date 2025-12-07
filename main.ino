#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// === LED STRIP SETUP ===
#define LED_PIN        15
#define NUM_LEDS       15
#define PIR_SIGNAL     5
#define BUTTON_MODE    4

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
bool ledState = false;
bool staticMode = false;
bool lastPirState = LOW;
bool lastModeButtonState = HIGH;

unsigned long previousMillis = 0;
const unsigned long interval = 200;
int step = 0;
bool turningOn = true;

// === OLED SETUP ===
#define SDA_PIN              10
#define SCL_PIN               9
#define TCA_ADDR           0x70
const uint8_t CHANNELS[] = {7, 6, 5};
const char     LETTERS[] = {'M','P','E'};

#define OLED_ADDR          0x3C
#define SCREEN_WIDTH         128
#define SCREEN_HEIGHT         64
#define BAR_WIDTH             22
#define BAR_HEIGHT             8
#define SLIDE_STEP_PIX         2
#define SLIDE_DELAY_MS        12
#define PAUSE_AFTER_ALL_ANIM 250
#define TEXT_SIZE              4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

uint8_t channelsMask() {
  uint8_t m = 0;
  for (uint8_t i = 0; i < sizeof(CHANNELS); i++) m |= (1 << CHANNELS[i]);
  return m;
}

void tcaSelect(uint8_t ch) {
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(ch <= 7 ? (1 << ch) : 0x00);
  Wire.endTransmission();
}

void tcaSelectMask(uint8_t mask) {
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(mask);
  Wire.endTransmission();
}

bool initDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR, false, false)) return false;
  display.clearDisplay();
  display.display();
  display.setTextColor(SSD1306_WHITE);
  return true;
}

void animateSlidingBar() {
  int y = (SCREEN_HEIGHT - BAR_HEIGHT) / 2;
  for (int x = -BAR_WIDTH; x <= SCREEN_WIDTH; x += SLIDE_STEP_PIX) {
    display.clearDisplay();
    display.fillRect(x, y, BAR_WIDTH, BAR_HEIGHT, SSD1306_WHITE);
    display.display();
    delay(SLIDE_DELAY_MS);
  }
  display.clearDisplay();
  display.display();
}

void drawCenteredCharToGDDR(char ch) {
  display.clearDisplay();
  display.setTextSize(TEXT_SIZE);

  int w = 6 * TEXT_SIZE;
  int h = 8 * TEXT_SIZE;
  int x = (SCREEN_WIDTH  - w) / 2;
  int y = (SCREEN_HEIGHT - h) / 2;

  display.setCursor(x, y);
  display.write(ch);
  display.display();
}

void setup() {
  // === OLED INIT + ANIMATION SEQUENCE ===
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);
  for (uint8_t i = 0; i < sizeof(CHANNELS); i++) {
    tcaSelect(CHANNELS[i]);
    if (!initDisplay()) { delay(50); initDisplay(); }
  }
  tcaSelect(255);

  for (uint8_t i = 0; i < sizeof(CHANNELS); i++) {
    tcaSelect(CHANNELS[i]);
    animateSlidingBar();
  }
  tcaSelect(255);

  uint8_t mask = channelsMask();
  tcaSelectMask(mask);
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  tcaSelect(255);

  const uint8_t n = min((size_t)sizeof(CHANNELS), (size_t)sizeof(LETTERS));
  for (uint8_t i = 0; i < n; i++) {
    tcaSelect(CHANNELS[i]);
    drawCenteredCharToGDDR(LETTERS[i]);
  }
  tcaSelect(255);

  delay(PAUSE_AFTER_ALL_ANIM);

  tcaSelectMask(mask);
  display.ssd1306_command(SSD1306_DISPLAYON);
  tcaSelect(255);

  // === LED STRIP INIT ===
  pinMode(PIR_SIGNAL, INPUT);
  pinMode(BUTTON_MODE, INPUT_PULLUP);
  strip.begin();
  strip.show();  // start off
}

void loop() {
  // --- PIR TRIGGER HANDLING ---
  bool pirState = digitalRead(PIR_SIGNAL);
  if (lastPirState == LOW && pirState == HIGH) {
    ledState = !ledState;
    delay(1000);  // debounce for PIR
  }
  lastPirState = pirState;

  // --- MODE BUTTON HANDLING ---
  bool modeButtonState = digitalRead(BUTTON_MODE);
  if (lastModeButtonState == HIGH && modeButtonState == LOW) {
    staticMode = !staticMode;
    delay(200);
  }
  lastModeButtonState = modeButtonState;

  // --- LED STRIP LOGIC ---
  if (ledState) {
    if (staticMode) {
      for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 128, 128)); // teal
      }
      strip.show();
    } else {
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        if (turningOn) {
          strip.setPixelColor(step, strip.Color(0, 128, 128));
          strip.show();
          step++;
          if (step >= NUM_LEDS) {
            turningOn = false;
            step = 0;
          }
        } else {
          strip.setPixelColor(step, strip.Color(0, 0, 0));
          strip.show();
          step++;
          if (step >= NUM_LEDS) {
            turningOn = true;
            step = 0;
          }
        }
      }
    }
  } else {
    strip.clear();
    strip.show();
  }
}
