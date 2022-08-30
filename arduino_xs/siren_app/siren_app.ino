// LoRa siren app Arduino Pro (https://github.com/Makerfabs/Makerfabs_MaLora/tree/main/06MOS4)


const int RFM95_CS = 10;
const int RFM95_RST = 4;
const int RFM95_INT = 2;
const float RF95_FREQ = 868.0;
const int LED_PIN = A3;
const int MOS_PINS[4] = {5, 6, 9, 3};
const int MOS_ALARM_PIN = MOS_PINS[0];
const int EEPROM_RUNCOUNT_ADDR = 0;
const uint32_t ALARM_TIMEOUT_MS = 5000;
const float REF_V = 3.3;
const int BTY_PIN = A9;
const float BTY_VRR = 0.5;
const float BTY_LOW_V = 12.1;
const float BTY_MIN_V = 12;
const float BTY_MAX_V = 12.2;

#include "config.h"
#include <mysmarthome.h>

#include <EEPROM.h>


bool radioInit = false;


// update correct alarm state
void updateSensorState(uint32_t _src, uint8_t _state)
{
  unsigned long timestamp = millis();
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    Sensor &sensor = sensors[i];
    if (sensor.address == _src)
    {
      if (sensor.state & MSG_STATE::SENSOR_XS::OPEN != _state & MSG_STATE::SENSOR_XS::OPEN)
      {
        sensor.timestamp = timestamp;
      }

      sensor.state = _state;
      break;
    }
  }
}

// check if alarm should be on
bool checkAlarm(unsigned long timeout)
{
  unsigned long timestamp = millis();
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    Sensor &sensor = sensors[i];
    if (sensor.state & MSG_STATE::SENSOR_XS::OPEN)
    {
      if (timestamp - sensor.timestamp < timeout)
      {
        return true;
      }
    }
  }

  return false;
}

// check if any sensor has a low battery state
bool checkBtyLow()
{
  unsigned long timestamp = millis();
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    Sensor &sensor = sensors[i];
    if (sensor.state & MSG_STATE::SENSOR_XS::BTYLOW)
    {
      return true;
    }
  }

  return false;
}

// receive and process radio messages
void recvRadioMessages()
{
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
        updateSensorState(msg.address, msg.state);
      }
    }
    else
    {
      // no more message
      break;
    }
  }
}

void setup() 
{
  // setup outputs
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  analogWrite(MOS_PINS[0], 255);
  analogWrite(MOS_PINS[1], 255);
  analogWrite(MOS_PINS[2], 255);
  analogWrite(MOS_PINS[3], 255);

  Serial.begin(9600);

  // setup and test LoRa
  radioInit = setupRadio();

  // increment run count
  EEPROM.write(EEPROM_RUNCOUNT_ADDR, EEPROM.read(0)+1);
  Serial.print("Run count = ");
  Serial.println(EEPROM.read(EEPROM_RUNCOUNT_ADDR));
  Serial.print("Num sensors = ");
  Serial.println(NUM_SENSORS);
}

void loop()
{
  static int runCount = EEPROM.read(0);
  
  if (!radioInit)
  {
    flashFast();
    delay(10);
  }
  else
  {  
    recvRadioMessages();
    if (checkAlarm(ALARM_TIMEOUT_MS))
    {
      analogWrite(MOS_ALARM_PIN, 0);
    }
    else
    {
      analogWrite(MOS_ALARM_PIN, 255);
    }

    // check other sensor states
    bool btyLow = checkBtyLow();
    
    // show app status
    if (btyLow)
    {
      flashFast();
    }
    else
    {    
      blink();
    }
    
    delay(randomByte() + 10);
  }
}
