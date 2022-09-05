
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
const unsigned long DSP_TIMEOUT_MS = 3000;
const int BUTTON1_PIN = A1;
const int BUTTON2_PIN = A2;

#include "config.h"
#include <mysmarthome.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
bool oledInit = false;
uint8_t dspIndex = 0;
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
  static const char symbols[] = {'|', '/', '-', '\\'};
  return symbols[msgCount % sizeof(symbols)];
}


// check if alarm should be on
bool checkAlarm(unsigned long timeout)
{
  unsigned long timestamp = millis();
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    Sensor &sensor = sensors[i];
    if (sensor.state & STATE::SENSOR_XS::OPEN)
    {
      if (timestamp - sensor.timestamp < timeout)
      {
        return true;
      }
    }
  }

  return false;
}


String getSensorString(const Sensor &_sensor)
{
  String str;
  str = _sensor.name;
  str += " - ";

  if (_sensor.state & STATE::SENSOR_XS::OPEN) str += "OPEN";
  else if (_sensor.state & STATE::SENSOR_XS::BTYLOW) str += "BTY LOW";
  else str += "CLOSED";

  return str;
}


// check if any sensor has a low battery state
bool checkBtyLow()
{
  unsigned long timestamp = millis();
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    Sensor &sensor = sensors[i];
    if (sensor.state & STATE::SENSOR_XS::BTYLOW)
    {
      return true;
    }
  }

  return false;
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


// receive and process radio messages
unsigned long recvRadioMessages()
{
  static unsigned long msgCount = 0;
  while (1)
  {
    // try to read next message
    if (Message msg; recvRadioMsg(msg, PROTO_ID) > 0)
    {
      Serial.print("rx: app=");
      Serial.print(msg.application);
      Serial.print(", addr=");
      Serial.print(msg.address);
      Serial.print(", state=");
      Serial.print(msg.state);
      Serial.print(", crc=");
      Serial.println(msg.crc);

      if (msg.application == (uint8_t)APPLICATION::SENSOR_XS)
      {
        Sensor &sensor = getSensorState(msg.address);
        if (sensor.name)
        {
          if ((sensor.state & STATE::SENSOR_XS::OPEN) != (msg.state & STATE::SENSOR_XS::OPEN))
          {
            sensor.timestamp = millis();
            Timer<0>::start(DSP_TIMEOUT_MS);

            if (msg.state & STATE::SENSOR_XS::OPEN)
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


#define DP(s) display.print(s);
#define DE(s) display.println(s);

void drawMain(unsigned long msgCount)
{
  static String dspMsg;

  // draw display/battery state
  display.setTextSize(1);
  display.setTextColor(WHITE);
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
  display.setTextColor(WHITE);

  if (!Timer<0>::hasExpired())
  {
    dspMsg = "";
    for (int i = 0; i < NUM_SENSORS; i++)
    {
      dspIndex++;
      const Sensor &sensor = sensors[dspIndex % NUM_SENSORS];
      if (sensor.name)
      {
        if (sensor.state & STATE::SENSOR_XS::OPEN)
        {
          Timer<0>::start(DSP_TIMEOUT_MS);
          dspMsg = sensor.name;
          break;
        }
      }
    }
  }
     
  DE(dspMsg.c_str());

  // draw last received message string
  display.setTextSize(1);
  DE("")DE("")DP(getMsgCountSymbol(msgCount))DP(" ")DP(lastMsg.c_str());
}


void drawList()
{
  display.setTextSize(0);
  display.setTextColor(BLACK, WHITE);
  display.fillScreen(WHITE);
  
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    String str = getSensorString(sensors[i]);
    DE(str);
  }
}


void setup() 
{
  // setup PINS
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  
  // setup serial
  Serial.begin(9600);
  
  // setup display
  oledInit = setupDisplay();
  display.clearDisplay();
  display.display();
  
  // setup and test LoRa
  setupRadio();
}


void loop()
{
  static bool alarm = false;
  static bool btyLow = false;

  if (!isRadioInitialized() || !oledInit)
  {
    flashFast();
  }
  else
  {
    // receive messages
    unsigned long msgCount = recvRadioMessages();

    // check sensor states
    alarm = checkAlarm(DSP_TIMEOUT_MS);
    btyLow = checkBtyLow();

    // prepare display display
    display.clearDisplay();
    display.cp437(true);
    display.setCursor(0, 0);

    if (!digitalRead(BUTTON1_PIN))
    {
      drawList();
    }
    else
    {
      drawMain(msgCount);
    }

    display.dim(true);
    display.display();

    // status
    if (alarm)
    {
      flash();
    }
    else if (btyLow)
    {
      blinkAndFlash();
    }
    else
    {
      blink();
    }
  }
  
  delay(randomByte()/50);
}
