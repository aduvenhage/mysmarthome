
// LoRa sensor dsiplay on 32u4 (adafruit feather, 868Mhz), with Adafruit ssd1306_128x64_spi

const int RFM95_CS = 8;
const int RFM95_RST = 4;
const int RFM95_INT = 7;
const float RF95_FREQ = 868.0;
const int LED_PIN = 13;
const int VBATPIN = A9;
const float BTY_LOW_V = 3.6;
const int SCREEN_WIDTH = 128; // OLED display width, in pixels
const int SCREEN_HEIGHT = 64; // OLED display height, in pixels
const int OLED_MOSI = 5;
const int OLED_CLK = 10;
const int OLED_DC = 11;
const int OLED_CS = 12;
const int OLED_RESET = 6;

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <mysmarthome.h>


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
bool radioInit = false;
bool oledInit = false;


float getBatteryVoltage()
{
  // NOTE: multiplying by 2 since hardware has a devider on the pin
  static float vbatt = 0;
  vbatt = 0.9*vbatt + 0.1*analogRead(VBATPIN) * 2.0 * 3.3 / 1024.0;
  return vbatt;
}

bool setupDisplay()
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC))
  {
    return false;
  }

  return true;
}

void setup() 
{
  // setup LEDS
  pinMode(LED_PIN, OUTPUT);
  
  // setup serial
  Serial.begin(9600);
  int wait = 0;
  while (!Serial && wait++ < 10)
  {
    delay(500);
  }
  
  // setup display
  oledInit = setupDisplay();
  display.clearDisplay();
  display.display();
  display.dim(true);

  
  // setup and test LoRa
  radioInit = setupRadio();
}

void loop()
{
  static bool btyLow = false;

  if (!radioInit || !oledInit)
  {
    flashFast();
    delay(10);
  }
  else
  {
    // draw
    display.clearDisplay();

    display.cp437(true);
    display.setCursor(0, 0);

    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    
    float vbatt = getBatteryVoltage();
    display.print(vbatt); display.println("v");
    
    display.display();

    // status
    blink();
    delay(randomByte()/10 + 10);
  }
}
