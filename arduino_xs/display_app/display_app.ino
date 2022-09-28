
// LoRa sensor dsiplay on 32u4 (adafruit feather, 868Mhz), with Adafruit ssd1306_128x64_spi

constexpr int RFM95_CS = 8;
constexpr int RFM95_RST = 4;
constexpr int RFM95_INT = 7;
constexpr float RF95_FREQ = 868.0;
constexpr int LED_PIN = 13;
constexpr int LED_ON = HIGH;
constexpr float REF_V = 3.3;
constexpr int BTY_PIN = A9;
constexpr int CHG_PIN = A0;
constexpr float BTY_VRR = 0.5;
constexpr float CHG_VRR = 0.5;
constexpr float BTY_MIN_V = 3.5;
constexpr float BTY_LOW_V = 3.6;
constexpr float BTY_MAX_V = 4.18;
constexpr int SCREEN_WIDTH = 128; // OLED display width, in pixels
constexpr int SCREEN_HEIGHT = 64; // OLED display height, in pixels
constexpr int OLED_MOSI = 5;
constexpr int OLED_CLK = 10;
constexpr int OLED_DC = 11;
constexpr int OLED_CS = 12;
constexpr int OLED_RESET = 6;
constexpr int BUTTON1_PIN = A1;
constexpr int BUTTON2_PIN = A2;
constexpr int CYCLE_TIMER = 0;
constexpr int NODE_TIMER = 1;
constexpr int BT2_TIMER = 2;
constexpr int SLEEP_TIMER = 3;
constexpr unsigned long CYCLE_TIMEOUT = 3000;
constexpr unsigned long NODE_TIMEOUT = 4000;
constexpr unsigned long SLEEP_TIMEOUT = 30000;
constexpr unsigned long ALARM_TIMEOUT = 16000;

#include "config.h"
#include <mysmarthome.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
bool oledInit = false;
Node *lastMsg = nullptr;
Node *lastOpen = nullptr;
Node *lastBtyLow = nullptr;
Node *lastMuted = nullptr;
Node *dspNode = nullptr;


bool setupDisplay()
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  return display.begin(SSD1306_SWITCHCAPVCC);
}


char getMsgCountSymbol(uint8_t msgCount)
{
  static const char symbols[] = {'|', '/', '-', '\\'};
  return symbols[msgCount % sizeof(symbols)];
}


// find sensor state
Node *getNode(uint32_t _src)
{
  for (uint8_t i = 0; i < NUM_NODES; i++)
  {
    Node &node = nodes[i];
    if (node.address == _src)
    {
      return &node;
    }
  }

  return nullptr;
}


// receive and process radio messages
uint8_t recvRadioMessages()
{
  static uint8_t msgCount = 0;
  while (1)
  {
    // try to read next message
    if (Message msg; recvRadioMsg(msg, PROTO_ID) > 0)
    {
      ssprintf(Serial, "rx: %s, addr=%lu, state=%d, crc=%d\n", msg.application, msg.address, (int)msg.state, (int)msg.crc);

      if (msg.application == APPLICATION::NODE)
      {
        Node *node = getNode(msg.address);
        if (node)
        {
          // check for events
          if (STATE::hasFlag(msg.state, STATE::NODE::OPEN)) lastOpen = node;
          if (STATE::hasFlag(msg.state, STATE::NODE::BTYLOW)) lastBtyLow = node;
          if (STATE::hasFlag(msg.state, STATE::NODE::MUTED)) lastMuted = node;

          // reset node timestamp only on new alarm
          if (STATE::hasFlag(node->state, STATE::NODE::OPEN) != STATE::hasFlag(msg.state, STATE::NODE::OPEN))
          {
            dspNode = nullptr;
            node->timestamp = millis();
            Timer<CYCLE_TIMER>::start(CYCLE_TIMEOUT);
          }
          
          // update rx node state
          node->state = msg.state;
          lastMsg = node;
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


bool isNodeMuted()
{
  return lastMuted ? STATE::hasFlag(lastMuted->state, STATE::NODE::MUTED) : false;
}


bool isNodeBtyLow()
{
  return lastBtyLow ? STATE::hasFlag(lastBtyLow->state, STATE::NODE::BTYLOW) : false;
}


bool isNodeOpen()
{
  return lastOpen ? STATE::hasFlag(lastOpen->state, STATE::NODE::OPEN) || (millis() - lastOpen->timestamp < ALARM_TIMEOUT) : false;
}


void drawMain(unsigned long msgCount)
{
  // draw display/battery state
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  ssprintf(display,
           " %c %s \t  bty%s %3d \n\n\n",
           getMsgCountSymbol(msgCount),
           isNodeMuted() ? "muted" : "     ",
           isBatteryCharging() ? "*" : "",
           (int)getBatteryPercentage());

  // draw current open sensor states 
  display.setTextSize(2);
  display.setTextColor(WHITE);

  if (dspNode)
  {
    ssprintf(display, "%s\n", dspNode->name);
  }
  else if (isNodeOpen())
  {
    ssprintf(display, "%s\n", lastOpen->name);
  }
  else
  {
    ssprintf(display, "\n");
  }
  
  // draw last received message string
  if (lastMsg)
  {
    display.setTextSize(1);
    ssprintf(display, "\n %s - %s\n", lastMsg->name, (STATE::NODE)lastMsg->state);
  }
  else
  {
    ssprintf(display, "\n\n");
  }

  // draw static button labels
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  ssprintf(display, "   mute   \t   list   ");
}


void drawList()
{
  display.setTextSize(0);
  display.setTextColor(BLACK, WHITE);
  display.fillScreen(WHITE);
  
  for (uint8_t i = 0; i < NUM_NODES; i++)
  {
    Node &node = nodes[i];
    ssprintf(display, "%s - %s\n", node.name, (STATE::NODE)node.state);
  }
}


bool isButtonPressed(int pin)
{
  return !digitalRead(pin);
}


bool isSleeping()
{
  return Timer<SLEEP_TIMER>::hasExpired();
}


void wakeUp()
{
  Timer<SLEEP_TIMER>::start(SLEEP_TIMEOUT);
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
  wakeUp();
}


void loop()
{
  static bool muted = false;
  static bool buttonsDown = false;
  static uint8_t dspIndex = 0;

  if (!isRadioInitialized() || !oledInit)
  {
    flashFast();
  }
  else
  {
    // receive messages
    uint8_t msgCount = recvRadioMessages();

    // reset alarm message and find next display message
    if (Timer<CYCLE_TIMER>::hasExpired())
    {
      dspNode = nullptr;
      for (uint8_t i = 0; i < NUM_NODES+1; i++)
      {
        dspIndex++;
        const Node &node = nodes[dspIndex % NUM_NODES];
        if (STATE::hasFlag(node.state, STATE::NODE::OPEN) || (millis() - node.timestamp < ALARM_TIMEOUT))
        {
          dspNode = &node;
          break;
        }
      }
      
      Timer<CYCLE_TIMER>::start(CYCLE_TIMEOUT);
      ssprintf(Serial, "DSP restart %s\n", dspNode != nullptr);      
    }

    // prepare display
    display.clearDisplay();
    display.cp437(true);
    display.setCursor(0, 0);

    if (isSleeping())
    {
      
    }
    else if (isButtonPressed(BUTTON1_PIN))
    {
      drawList();
    }
    else
    {
      drawMain(msgCount);
    }

    display.dim(true);
    display.display();

    // send out mute command
    if (!isSleeping() && isButtonPressed(BUTTON2_PIN))
    {
      if (Timer<BT2_TIMER>::hasExpired())
      {
        muted = !muted;
        sendCmdMsg(muted);
        Timer<BT2_TIMER>::start(1000);

        ssprintf(Serial, "tx: muted=%s", muted);
      }
    }

    // get node state
    bool charging = isBatteryCharging();
    bool btyLow = isBatteryLow();
    bool alarm = isNodeOpen();
    bool nodesBtyLow = isNodeBtyLow();

    // send out node state
    if (Timer<NODE_TIMER>::hasExpired())
    {
      sendNodeMsg(false, isBatteryLow, charging, false);
      Timer<NODE_TIMER>::start(NODE_TIMEOUT - randomByte()*4);
      
      ssprintf(Serial, "tx: vbatt=%d, bty_low=%s, charging=%s\n", (int)(getBatteryVoltage()*100.0f + 0.5f), btyLow, charging);
    }

    // wake up display on alarm or button press
    if ((alarm) || (!isButtonPressed(BUTTON2_PIN) && !isButtonPressed(BUTTON1_PIN) && buttonsDown))
    {
      wakeUp();
    }

    buttonsDown = isButtonPressed(BUTTON2_PIN) || isButtonPressed(BUTTON1_PIN);

    // status
    if (alarm)
    {
      flash();
    }
    else if (btyLow || nodesBtyLow)
    {
      blinkAndFlash();
    }
    else if (!charging)
    {
      blink();
    }
    else
    {
      digitalWrite(LED_PIN, LED_ON);
    }
  }
}
