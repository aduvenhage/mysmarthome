
// LoRa sensor on 32u4 (adafruit feather, 868Mhz)

// framework constans/setup
constexpr int RFM95_CS = 8;
constexpr int RFM95_RST = 4;
constexpr int RFM95_INT = 7;
constexpr float RF95_FREQ = 868.0;
constexpr int INPUT_PIN = 6;
constexpr int LED_PIN = 13;
constexpr int LED_ON = HIGH;
constexpr float REF_V = 3.3;
constexpr int BTY_PIN = A9;
constexpr int CHG_PIN = A0;
constexpr float BTY_VRR = 0.5;
constexpr float CHG_VRR = 0.5;
constexpr float BTY_MIN_V = 3.5;
constexpr float BTY_LOW_V = 3.6;
constexpr float BTY_MAX_V = 4.2;
constexpr unsigned long MSG_TIMEOUT = 4000;
constexpr int NODE_TIMER = 0;

#include "config.h"
#include <mysmarthome.h>


bool isSensorOpen()
{
  return digitalRead(INPUT_PIN);
}

void setup() 
{
  // setup PINS
  pinMode(INPUT_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  // setup serial
  Serial.begin(9600);

  // setup LoRa
  setupRadio();
}

void testXS()
{
  for (;;)
  {
    uint32_t t0 = millis();
    while (millis() - t0 < 2000)
    {
      flashFast();
    }
    
    for (int i = 0; i < 5; i++)
    {
      sendNodeMsg(true, false, false, false, 'HEKM');
      flipLed();
      delay(500);
      sendNodeMsg(true, false, false, false, 'VOOR');
      flipLed();
      delay(4000);
    }
    
    for (int i = 0; i < 5; i++)
    {
      sendNodeMsg(true, false, false, false, 'HEKM');
      flipLed();
      delay(500);
      sendNodeMsg(false, false, false, false, 'VOOR');
      flipLed();
      delay(4000);
    }

    for (int i = 0; i < 5; i++)
    {
      sendNodeMsg(false, false, false, false, 'HEKM');
      flipLed();
      delay(500);
      sendNodeMsg(false, false, false, false, 'VOOR');
      flipLed();
      delay(4000);
    }
  }
}

void loop()
{
  testXS();

  
  static bool sensorOpen = false;
  static bool btyLow = false;
  static bool charging = false;

  if (!isRadioInitialized())
  {
    // error state
    flashFast();
  }
  else
  {
    btyLow = isBatteryLow();
    charging = isBatteryCharging();

    if (isSensorOpen() != sensorOpen)
    {
      // restart timer to send new node state
      sensorOpen = isSensorOpen();
      Timer<NODE_TIMER>::start(randomByte()/10);
    }
    
    if (Timer<NODE_TIMER>::hasExpired())
    {
      // send node state
      sendNodeMsg(sensorOpen, btyLow, charging, false);
      ssprintf(Serial, "tx: open=%s, vbatt=%d, bty_low=%s, charging=%s\n", sensorOpen, (int)(getBatteryVoltage()*100.0f + 0.5f), btyLow, charging);

      // restart timer
      Timer<NODE_TIMER>::start(MSG_TIMEOUT - randomByte()*4ul);
    }
    
    if (sensorOpen)
    {
      flash();
    }
    else
    {
      blink();
    }
  }
}
