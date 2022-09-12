
// LoRa sensor on 32u4 (adafruit feather, 868Mhz)

// framework constans/setup
constexpr int RFM95_CS = 8;
constexpr int RFM95_RST = 4;
constexpr int RFM95_INT = 7;
constexpr float RF95_FREQ = 868.0;
constexpr int INPUT_PIN = 6;
constexpr int LED_PIN = 13;
constexpr int LED_ON = LOW;
constexpr float REF_V = 3.3;
constexpr int BTY_PIN = A9;
constexpr int CHG_PIN = A0;
constexpr float BTY_VRR = 0.5;
constexpr float CHG_VRR = 0.5;
constexpr float BTY_MIN_V = 3.5;
constexpr float BTY_LOW_V = 3.6;
constexpr float BTY_MAX_V = 4.2;
constexpr unsigned long MSG_TIMEOUT_MS = 4000;
constexpr int NODE_TIMER = 0;

#include "config.h"
#include <mysmarthome.h>


bool isSensorOpen()
{
  return !digitalRead(INPUT_PIN);
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

void loop()
{
  static bool sensorOpen = false;
  static bool btyLow = false;
  static bool charging = false;

  if (!isRadioInitialized())
  {
    flashFast();
  }
  else
  {
    btyLow = isBatteryLow();
    charging = isBatteryCharging();
    
    if (Timer<NODE_TIMER>::hasExpired() || (isSensorOpen() != sensorOpen))
    {
      Timer<NODE_TIMER>::start(MSG_TIMEOUT_MS + randomByte()*4);
      sensorOpen = isSensorOpen();
      sendNodeMsg(sensorOpen, btyLow, charging, false);

      ssprintf(Serial, "tx: open=%s, vbatt=%.2f, bty_low=%s, charging=%s", sensorOpen, getBatteryVoltage(), btyLow, charging);
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
  
  delay(randomByte()/50);
}
