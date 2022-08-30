
// LoRa sensor on 32u4 (adafruit feather, 868Mhz)

const int RFM95_CS = 8;
const int RFM95_RST = 4;
const int RFM95_INT = 7;
const float RF95_FREQ = 868.0;
const int INPUT_PIN = 6;
const int LED_PIN = 13;
const int MSG_TIMEOUT_MS = 4000; 
const int VBATPIN = A9;
const float BTY_LOW_V = 3.6;

#include <mysmarthome.h>


bool radioInit = false;


bool isSensorOpen()
{
  return !digitalRead(INPUT_PIN);
}

float getBatteryVoltage()
{
  // NOTE: multiplying by 2 since hardware has a devider on the pin
  return analogRead(VBATPIN) * 2.0 * 3.3 / 1024.0;
}

void sendSensorMsg(bool _open, bool _btyLow)
{
  Message msg;
  msg.address = 1;
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
    if ((timeNow - timeMsg > MSG_TIMEOUT_MS) || (isSensorOpen() != sensorOpen))
    {
      timeMsg = timeNow;
      msgTimeout = MSG_TIMEOUT_MS + randomByte()*4;
      
      sensorOpen = isSensorOpen();
      btyLow = getBatteryVoltage() < BTY_LOW_V;
      sendSensorMsg(sensorOpen, btyLow);
      
      Serial.print(sensorOpen ? "Radio message: open, " : "Radio message: closed, ");
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
