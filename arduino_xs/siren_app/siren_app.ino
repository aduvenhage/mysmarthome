// LoRa siren app Arduino Pro (https://github.com/Makerfabs/Makerfabs_MaLora/tree/main/06MOS4)


const int RFM95_CS = 10;
const int RFM95_RST = 4;
const int RFM95_INT = 2;
const float RF95_FREQ = 868.0;
const int LED_PIN = A3;
const int MOS_PINS[4] = {5, 6, 9, 3};
const int MOS_ALARM_PIN = MOS_PINS[0];
const int EEPROM_RUNCOUNT_ADDR = 0;
const uint32_t ALARM_TIMEOUT_MS = 10000;
const float REF_V = 3.3;
const int BTY_PIN = A1;
const int CHG_PIN = A0;
const float BTY_VRR = 0.5;
const float CHG_VRR = 0.5;
const float BTY_LOW_V = 12.1;
const float BTY_MIN_V = 12;
const float BTY_MAX_V = 12.2;
const unsigned long MUTE_TIMEOUT = 1000ul*60ul*60ul;

#include "config.h"
#include <mysmarthome.h>

#include <EEPROM.h>


// receive and process radio messages
void recvRadioMessages()
{
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
          }
    
          sensor.state = msg.state;
        }
      }
      else if (msg.application == (uint8_t)APPLICATION::CONTROL)
      {
        if (msg.state == STATE::CONTROL::MUTE)
        {
          Timer<0>::start(MUTE_TIMEOUT);
        }
      }
    }
    else
    {
      // no more message
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


void setup() 
{
  // setup PINS
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
  EEPROM.write(EEPROM_RUNCOUNT_ADDR, EEPROM.read(EEPROM_RUNCOUNT_ADDR)+1);
  Serial.print("Run count = ");
  Serial.println(EEPROM.read(EEPROM_RUNCOUNT_ADDR));
  Serial.print("Num sensors = ");
  Serial.println(NUM_SENSORS);

  // check mute state
  bool muted = (bool)(EEPROM.read(EEPROM_RUNCOUNT_ADDR) % 2);
  if (muted)
  {
    Timer<0>::start(MUTE_TIMEOUT);
    Serial.println("muted = true");
  }
  else
  {
    Serial.println("muted = false");
  }
}

void loop()
{
  static bool alarm = false;
  static bool btyLow = false;
  static bool muted = false;
  
  if (!radioInit)
  {
    flashFast();
  }
  else
  {      
    // process radio messages
    recvRadioMessages();
    
    // check sensor states
    alarm = checkAlarm(ALARM_TIMEOUT_MS);
    btyLow = checkBtyLow();
    muted = !Timer<0>::hasExpired();

    if (alarm && !muted)
    {
      analogWrite(MOS_ALARM_PIN, 0);
    }
    else
    {
      analogWrite(MOS_ALARM_PIN, 255);
    }

    // show app status
    if (alarm)
    {
      flash();
    }
    else if (btyLow || muted)
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
