#define SDA_PIN 9
#define SCL_PIN 10

#define TCA_ADDR 0x70
#define OLED_ADDR 0x3C

#define W 128
#define H 64

Adafruit_SSD1306 leftOLED(W, H, &Wire, -1);
Adafruit_SSD1306 rightOLED(W, H, &Wire, -1);

void tcaSelect(uint8_t channel) {
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void drawCentered(Adafruit_SSD1306 &d, const char *txt) {
  d.clearDisplay();
  d.setTextSize(6);
  d.setTextColor(SSD1306_WHITE);
  d.setCursor(45, 16);
  d.print(txt);
  d.display();
}

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);

  // init left on channel 0
  tcaSelect(0);
  leftOLED.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  drawCentered(leftOLED, "D");

  // init right on channel 1
  tcaSelect(1);
  rightOLED.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  drawCentered(rightOLED, "L");
}

void loop() {}
