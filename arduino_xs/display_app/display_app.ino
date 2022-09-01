
// LoRa sensor dsiplay on 32u4 (adafruit feather, 868Mhz), with Adafruit ssd1306_128x64_spi

const int RFM95_CS = 8;
const int RFM95_RST = 4;
const int RFM95_INT = 7;
const float RF95_FREQ = 868.0;
const int LED_PIN = 13;
const float REF_V = 3.3;
const int BTY_PIN = A9;
const int CHG_PIN = A0;
const float BTY_VRR = 0.5;
const float CHG_VRR = 0.5;
const float BTY_MIN_V = 3.5;
const float BTY_MAX_V = 4.18;
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
uint8_t dspIndex = 0;
unsigned long dspTimestamp = 0;
String dspMsg;
String lastMsg;


bool setupDisplay()
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC))
  {
    return false;
  }

  return true;
}

char getMsgCountSymbol(int msgCount)
{
  const char symbols[] = {'|', '/', '-', '\\'};
  return symbols[msgCount % sizeof(symbols)];
}

// find sensor state
Sensor &getSensorState(uint32_t _src)
{
  static Sensor empty = {nullptr, 0, 0, 0};
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    Sensor &sensor = sensors[i];
    if (sensor.address == _src)
    {
      return sensor;
    }
  }

  return empty;
}

String getSensorString(const Sensor &_sensor)
{
  String str;
  str = _sensor.name;
  str += ", ";

  if (_sensor.state & MSG_STATE::SENSOR_XS::OPEN) str += "OPEN";
  else if (_sensor.state & MSG_STATE::SENSOR_XS::BTYLOW) str += "BTY LOW";
  else str += "CLOSED";

  return str;
}

// receive and process radio messages
unsigned long recvRadioMessages()
{
  static unsigned long msgCount = 0;
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
        Sensor &sensor = getSensorState(msg.address);
        if (sensor.name)
        {
          unsigned long timestamp = millis();
          if (sensor.state & MSG_STATE::SENSOR_XS::OPEN != msg.state & MSG_STATE::SENSOR_XS::OPEN)
          {
            sensor.timestamp = timestamp;
            dspTimestamp = timestamp;

            if (msg.state & MSG_STATE::SENSOR_XS::OPEN)
            {
              dspMsg = sensor.name;
            }
          }
      
          sensor.state = msg.state;
          lastMsg = getSensorString(sensor);
        }
      }

      msgCount++;
    }
    else
    {
      // no more message
      break;
    }
  }

  return msgCount;
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
  static String dspMsg;

  if (!radioInit || !oledInit)
  {
    flashFast();
    delay(10);
  }
  else
  {
    // receive messages
    unsigned long msgCount = recvRadioMessages();
    unsigned long timestamp = millis();

    // prepare display display
    display.clearDisplay();
    display.cp437(true);
    display.setCursor(0, 0);

    // draw display/battery state
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    if (isBatteryCharging())
    {
      DP("bty* ")DP((int)getBatteryPercentage())DP("%")DE("")DE("");
    }
    else
    {
      DP("bty ")DP((int)getBatteryPercentage())DP("%")DE("")DE("");
    }

    // draw current open sensor states 
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    if (timestamp - dspTimestamp > 1000)
    {
      dspMsg = "";
      for (int i = 0; i < NUM_SENSORS; i++)
      {
        dspIndex++;
        const Sensor &sensor = sensors[dspIndex % NUM_SENSORS];
        if (sensor.state & MSG_STATE::SENSOR_XS::OPEN)
        {
          dspTimestamp = timestamp;
          dspMsg = sensor.name;
          break;
        }
      }
    }
       
    DE(dspMsg.c_str());

    // draw last received message string
    display.setTextSize(1);
    DE("")DE("")DP(getMsgCountSymbol(msgCount))DP(" ")DP(lastMsg.c_str());
    display.display();

    // status
    blink();
    delay(randomByte()/10 + 10);
  }
}
