
// LoRa sensor dsiplay on 32u4 (adafruit feather, 868Mhz), with Adafruit ssd1306_128x64_spi

constexpr int RFM95_CS = 8;
constexpr int RFM95_RST = 4;
constexpr int RFM95_INT = 7;
constexpr float RF95_FREQ = 868.0;
constexpr int GND_PINS[] = {A0, A1, A3};
constexpr int LED_PIN = 13;
constexpr int LED_ON = HIGH;
constexpr float REF_V = 3.3;
constexpr int BTY_PIN = A9;
constexpr int CHG_PIN = A2;
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
constexpr int BUTTON1_PIN = A4;
constexpr int BUTTON2_PIN = A5;
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
#include <U8g2lib.h>


Node *lastMsg = nullptr;
Node *lastOpen = nullptr;
Node *lastBtyLow = nullptr;
Node *lastMuted = nullptr;
Node *dspNode = nullptr;

U8G2_SSD1306_128X64_NONAME_2_4W_SW_SPI display(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_CS, OLED_DC, OLED_RESET);


// print into display (support \n line breaks)
struct Printer
{
  int ypos = 0;
  void print(const char *str)
  {
    const char *p = str;
    while (*p)
    {
      if (*p == '\n' || ypos == 0)
      {
        ypos += display.getMaxCharHeight();
        display.setCursor(0, ypos);
      }
      
      display.print(*p);
      ++p;
    }
  }

  void reset() {ypos = 0;}
};    

Printer printer;


void setFont1X()
{
  // see https://github.com/olikraus/u8g2/wiki/fntgrpx11 for info on fonts, memory usage, etc
  display.setFont(u8g2_font_6x10_mr);
}

void setFont2X()
{
  // see https://github.com/olikraus/u8g2/wiki/fntgrpx11 for info on fonts, memory usage, etc
  display.setFont(u8g2_font_10x20_mr);
}

void setColorNormal()
{
  display.setDrawColor(1);
}

void setColorInverted()
{
  display.setDrawColor(0);
}

char getMsgCountSymbol(uint8_t _msgCount)
{
  static const char symbols[] = {'|', '/', '-', '\\'};
  return symbols[_msgCount % sizeof(symbols)];
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
  printer.reset();
  setFont1X();
  setColorInverted();
  
  ssprintf(printer,
           " %c %s :  bty%s %3d   \n\n",
           getMsgCountSymbol(msgCount),
           isNodeMuted() ? "muted" : "     ",
           isBatteryCharging() ? "*" : "",
           (int)getBatteryPercentage());

  // draw current open sensor states
  setFont2X();
  setColorNormal();
  if (dspNode)
  {
    ssprintf(printer, "  %s", dspNode->name);
  }
  else if (isNodeOpen())
  {
    ssprintf(printer, "  %s", lastOpen->name);
  }
  
  // draw last received message string
  setFont1X();
  if (lastMsg)
  {
    ssprintf(printer, "\n\n %s - %s\n", lastMsg->name, (STATE::NODE)lastMsg->state);
  }
  else
  {
    ssprintf(printer, "\n\n\n");
  }

  // draw static button labels
  setFont1X();
  setColorInverted();
  ssprintf(printer, "   mute   :   list   ");
}


void drawList()
{
  printer.reset();
  setFont1X();
  setColorInverted();
  
  for (uint8_t i = 0; i < NUM_NODES; i++)
  {
    Node &node = nodes[i];
    ssprintf(printer, "%s - %s           \n", node.name, (STATE::NODE)node.state);
  }
}


bool isButtonPressed(int pin)
{
  return !digitalRead(pin);
}


bool isSleeping()
{
  bool sleeping = Timer<SLEEP_TIMER>::hasExpired();
  if (sleeping)
  {
    display.setPowerSave(1);
  }
}


void wakeUp()
{
  display.setPowerSave(0);
  Timer<SLEEP_TIMER>::start(SLEEP_TIMEOUT);
}


void setup() 
{
  // setup PINS
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  // setup additional GND lines
  for (int pin : GND_PINS)
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  
  // setup serial
  Serial.begin(9600);
  
  // setup display
  display.begin();
  
  // setup and test LoRa
  setupRadio();
  wakeUp();
}


void loop()
{
  static bool muted = false;
  static bool buttonsDown = false;
  static uint8_t dspIndex = 0;

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
  if (isSleeping())
  {
  }
  else if (isButtonPressed(BUTTON1_PIN))
  {
    display.firstPage();
    do {
      drawList();
    }
    while (display.nextPage());
  }
  else
  {
    display.firstPage();
    do {
      drawMain(msgCount);
    }
    while (display.nextPage());
  }

  // send out mute command
  if (!isSleeping() && isButtonPressed(BUTTON2_PIN))
  {
    if (Timer<BT2_TIMER>::hasExpired())
    {
      muted = !muted;
      sendCmdMsg(muted);
      Timer<BT2_TIMER>::start(1000);

      ssprintf(Serial, "tx: muted=%s\n", muted);
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
