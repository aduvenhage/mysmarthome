
// LoRa sensor on 32u4 (adafruit feather, 868Mhz)

const int RFM95_CS = 8;
const int RFM95_RST = 4;
const int RFM95_INT = 7;
const float RF95_FREQ = 868.0;
const int INPUT_PIN = 6;
const int LED_PIN = 13;
const int MSG_TIMEOUT_MS = 4000; 
const float REF_V = 3.3;
const int BTY_PIN = A9;
const int CHG_PIN = A0;
const float BTY_VRR = 0.5;
const float CHG_VRR = 0.5;
const float BTY_MIN_V = 3.5;
const float BTY_MAX_V = 4.2;

#include "config.h"
#include <mysmarthome.h>


bool isSensorOpen()
{
  return !digitalRead(INPUT_PIN);
}

void sendSensorMsg(bool _open, bool _btyLow, bool _charging)
{
  Message msg;
  msg.address = ADDR;
  msg.state = _open ? STATE::SENSOR_XS::OPEN : 0;
  msg.state |= _btyLow ? STATE::SENSOR_XS::BTYLOW : 0;
  msg.state |= _charging ? STATE::SENSOR_XS::CHARGING : 0;

  sendRadioMsg(msg);
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
    
    if (Timer<0>::hasExpired() || (isSensorOpen() != sensorOpen))
    {
      Timer<0>::start(MSG_TIMEOUT_MS + randomByte()*4);
      sensorOpen = isSensorOpen();

      sendSensorMsg(sensorOpen, btyLow, charging);
      
      Serial.print(sensorOpen ? "tx: open, " : "tx: closed, ");
      Serial.print(getBatteryVoltage());
      Serial.print(", ");
      Serial.print(btyLow ? "bty_low=true" : "bty_low=false");
      Serial.print(", ");
      Serial.println(charging ? "charging=true" : "charging=false");
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
