
// LoRa sensor dsiplay on 32u4 (adafruit feather, 868Mhz), with Adafruit ssd1306_128x64_spi

const int RFM95_CS = 8;
const int RFM95_RST = 4;
const int RFM95_INT = 7;
const float RF95_FREQ = 868.0;
const int LED_PIN = 13;
const float REF_V = 3.3;
const int BTY_PIN = A9;
const float BTY_VRR = 0.5;
const float BTY_MIN_V = 3.5;
const float BTY_MAX_V = 4.2;
const int SCREEN_WIDTH = 128; // OLED display width, in pixels
const int SCREEN_HEIGHT = 64; // OLED display height, in pixels
const int OLED_MOSI = 5;
const int OLED_CLK = 10;
const int OLED_DC = 11;
const int OLED_CS = 12;
const int OLED_RESET = 6;

#include "config.h"
#include <mysmarthome.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
bool radioInit = false;
bool oledInit = false;
Message lastSensorMsg = {};


bool setupDisplay()
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC))
  {
    return false;
  }

  return true;
}

// update correct alarm state
void updateSensorState(uint32_t _src, uint8_t _state)
{
  unsigned long timestamp = millis();
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    Sensor &sensor = sensors[i];
    if (sensor.address == _src)
    {
      if (sensor.state & MSG_STATE::SENSOR_XS::OPEN != _state & MSG_STATE::SENSOR_XS::OPEN)
      {
        sensor.timestamp = timestamp;
      }

      sensor.state = _state;
      break;
    }
  }
}

const char *getSensorName(uint32_t _src)
{
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    Sensor &sensor = sensors[i];
    if (sensor.address == _src)
    {
      return sensor.name;
    }
  }

  return "";
}

String getSensorState(uint32_t _src)
{
  String str;
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    Sensor &sensor = sensors[i];
    if (sensor.address == _src)
    {
      str = sensor.name;
      str += ", ";

      if (sensor.state & MSG_STATE::SENSOR_XS::OPEN) str += "OPEN";
      else if (sensor.state & MSG_STATE::SENSOR_XS::BTYLOW) str += "BTY LOW";
      else str += "CLOSED";
    }
  }

  return str;
}

// receive and process radio messages
void recvRadioMessages()
{
  while (1)
  {
    // try to read next message
    if (Message msg; recvRadioMsg(msg, PROTO_ID) > 0)
    {
      Serial.print("message: app=");
      Serial.print(msg.application);
      Serial.print(", addr=");
      Serial.print(msg.address);
      Serial.print(", state=");
      Serial.print(msg.state);
      Serial.print(", crc=");
      Serial.println(msg.crc);

      if (msg.application == (uint8_t)MSG_APPLICATION::SENSOR_XS)
      {
        updateSensorState(msg.address, msg.state);
        lastSensorMsg = msg;
      }
    }
    else
    {
      // no more message
      break;
    }
  }
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


#define DP(s) display.print(s);
#define DE(s) display.println(s);

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
    // receive messages
    recvRadioMessages();

    // update display
    display.clearDisplay();

    display.cp437(true);
    display.setCursor(0, 0);

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);    
    DP("bty: ")DP((int)getBatteryPercentage())DP("% (")DP(getBatteryVoltage())DE("v)")DE("")DE("");
    
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);    
    DE("HEK MOTOR");

    display.setTextSize(1);
    DE("")DE("")DP(getSensorState(lastSensorMsg.address).c_str())DE("");

    display.display();

    // status
    blink();
    delay(randomByte()/10 + 10);
  }
}
