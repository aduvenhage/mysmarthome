
const int RFM95_CS = 10;
const int RFM95_RST = 4;
const int RFM95_INT = 2;
const float RF95_FREQ = 868.0;
const int LED_PIN = A3;
const int MOS_PINS[4] = {5, 6, 9, 3};
const int MOS_ALARM_PIN = MOS_PINS[0];
const int EEPROM_RUNCOUNT_ADDR = 0;
bool radioInit = false;

#include <EEPROM.h>
#include <mysmarthome.h>
#include "programming.h"

// update correct alarm state
void updateState(uint32_t _src, uint8_t _state)
{
  unsigned long timestamp = millis();
  for (int i = 0; i < NUM_SENSORS; i++)
  {
    Sensor &sensor = sensors[i];
    if (sensor.address == _src)
    {
      if (sensor.state & SENSOR_STATE_OPEN != _state & SENSOR_STATE_OPEN)
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
    if (sensor.state & SENSOR_STATE_OPEN)
    {
      if (timestamp - sensor.timestamp < timeout)
      {
        return true;
      }
    }
  }

  return false;
}

void recvRadioMessages()
{
  while (1)
  {
    // try to read next message
    if (Message msg; recvRadioMsg(msg, PROTO_ID) > 0)
    {
      if (msg.application == EApplication::LoraBase)
      {
        Serial.print("message: src_addr=");
        Serial.print(msg.address);
        Serial.print(", state=");
        Serial.print(msg.state);
        Serial.print(", crc=");
        Serial.println(msg.crc);
  
        updateState(msg.address, msg.state);
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
    if (checkAlarm(2000))
    {
      analogWrite(MOS_ALARM_PIN, 0);
    }
    else
    {
      analogWrite(MOS_ALARM_PIN, 255);
    }
    
    blink();
    delay(10);
  }
}
