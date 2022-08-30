
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
const float BTY_VRR = 0.5;
const float BTY_MIN_V = 3.5;
const float BTY_MAX_V = 4.2;

#include "config.h"
#include <mysmarthome.h>


bool radioInit = false;


bool isSensorOpen()
{
  return !digitalRead(INPUT_PIN);
}

void sendSensorMsg(bool _open, bool _btyLow)
{
  Message msg;
  msg.address = ADDR;
  msg.state = _open ? MSG_STATE::SENSOR_XS::OPEN : 0;
  msg.state |= _btyLow ? MSG_STATE::SENSOR_XS::BTYLOW : 0;

  sendRadioMsg(msg);
}

void setup() 
{
  // setup inputs
  pinMode(INPUT_PIN, INPUT_PULLUP);

  // setup LEDS
  pinMode(LED_PIN, OUTPUT);
  
  // setup serial
  Serial.begin(9600);
  int wait = 0;
  while (!Serial && wait++ < 10)
  {
    delay(200);
  }

  // setup and test LoRa
  radioInit = setupRadio();
}

void loop()
{
  static bool sensorOpen = false;
  static bool btyLow = false;
  static unsigned long timeMsg = 0;
  unsigned long timeNow = millis();
  unsigned long msgTimeout = MSG_TIMEOUT_MS;

  if (!radioInit)
  {
    flashFast();
    delay(10);
  }
  else
  {
    btyLow = isBatteryLow();
    sensorOpen = isSensorOpen();

    if ((timeNow - timeMsg > MSG_TIMEOUT_MS) || (isSensorOpen() != sensorOpen))
    {
      timeMsg = timeNow;
      msgTimeout = MSG_TIMEOUT_MS + randomByte()*4;
      sendSensorMsg(sensorOpen, btyLow);
      
      Serial.print(sensorOpen ? "Radio message: open, " : "Radio message: closed, ");
      Serial.print(getBatteryVoltage());
      Serial.print(", ");
      Serial.println(btyLow ? "bty_low=true" : "bty_low=false");
    }
    
    if (sensorOpen)
    {
      digitalWrite(LED_PIN, HIGH);
    }
    else
    {
      blink();
    }

    delay(randomByte()/10 + 10);
  }
}
