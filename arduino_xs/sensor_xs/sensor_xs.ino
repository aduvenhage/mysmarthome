
const int RFM95_CS = 8;
const int RFM95_RST = 4;
const int RFM95_INT = 7;
const float RF95_FREQ = 868.0;
const int INPUT_PIN = 6;
const int LED_PIN = 13;
bool radioInit = false;

#include <mysmarthome.h>


bool isSensorOpen()
{
  return !digitalRead(INPUT_PIN);
}

void sendSensorMsg(bool _open)
{
  Message msg;
  msg.application = EApplication::LoraBase;
  msg.address = 1;
  msg.state = _open ? SENSOR_STATE_OPEN : 0;

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
  static unsigned long timeMsg = 0;
  unsigned long timeNow = millis();

  if (!radioInit)
  {
    flashFast();
    delay(10);
  }
  else
  {
    if ((timeNow - timeMsg > 2000) || (isSensorOpen() != sensorOpen))
    {
      timeMsg = timeNow;
      sensorOpen = isSensorOpen();
      sendSensorMsg(sensorOpen);
      Serial.println(sensorOpen ? "Radio message: open" : "Radio message: closed");
    }
    
    if (sensorOpen)
    {
      digitalWrite(LED_PIN, HIGH);
    }
    else
    {
      blink();
    }

    delay(10);
  }
}
