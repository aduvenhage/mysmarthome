// LoRa siren app Arduino Pro (https://github.com/Makerfabs/Makerfabs_MaLora/tree/main/06MOS4)


constexpr int RFM95_CS = 10;
constexpr int RFM95_RST = 4;
constexpr int RFM95_INT = 2;
constexpr float RF95_FREQ = 868.0;
constexpr int LED_PIN = A3;
constexpr int LED_ON = LOW;
constexpr int MOS_PINS[4] = {5, 6, 9, 3};
constexpr int MOS_ALARM_ALL_PIN = MOS_PINS[0];
constexpr int MOS_ALARM_MUTED_PIN = MOS_PINS[1];
constexpr int MOS_ALARM_NORMAL_PIN = MOS_PINS[2];
constexpr uint32_t ALARM_TIMEOUT_MS = 10000;
constexpr float REF_V = 3.3;
constexpr int BTY_PIN = A1;
constexpr int CHG_PIN = A0;
constexpr float BTY_VRR = 0.5;
constexpr float CHG_VRR = 0.5;
constexpr float BTY_MIN_V = 12;
constexpr float BTY_LOW_V = 12.1;
constexpr float BTY_MAX_V = 12.2;
constexpr unsigned long MUTE_TIMEOUT = 1000ul*60ul*60ul;  // 1 hour
constexpr unsigned long MSG_TIMEOUT = 4000;
constexpr int MUTE_TIMER = 0;
constexpr int NODE_TIMER = 1;
constexpr int SIREN_TIMER = 2;


#include "config.h"
#include <mysmarthome.h>


// returns true if siren should be on
bool isSirenOn()
{
  return !Timer<SIREN_TIMER>::hasExpired();
}


// start siren timer
void startSiren(unsigned long _timeout)
{
  Timer<SIREN_TIMER>::start(_timeout);
}


// returns true if siren is muted
bool isMuted()
{
  return !Timer<MUTE_TIMER>::hasExpired();
}


// start mute timer
void startMute(unsigned long _timeout)
{
  Timer<MUTE_TIMER>::start(_timeout);
}


// find node state
Node &getNodeState(uint32_t _src)
{
  static Node empty = {nullptr, 0, 0, 0};
  for (uint8_t i = 0; i < NUM_NODES; i++)
  {
    Node &node = nodes[i];
    if (node.address == _src)
    {
      return node;
    }
  }

  return empty;
}


// receive and process radio messages
void recvRadioMessages()
{
  while (1)
  {
    // try to read next message
    if (Message msg; recvRadioMsg(msg, PROTO_ID) > 0)
    {
      ssprintf(Serial, "rx: app=%d, addr=%lu, state=%d, crc=%d\n", (int)msg.application, msg.address, (int)msg.state, (int)msg.crc);

      if (msg.application == APPLICATION::NODE)
      {
        Node &node = getNodeState(msg.address);
        if (node.name)
        {
          // start siren if new node state is open
          if (!STATE::hasFlag(node.state, STATE::NODE::OPEN) && STATE::hasFlag(msg.state, STATE::NODE::OPEN))
          {
            startSiren(ALARM_TIMEOUT_MS);
          }

          // update node state
          node.state = msg.state;
          node.timestamp = millis();
        }
      }
      else if (msg.application == APPLICATION::CONTROL)
      {
        // set new mute state
        startMute(STATE::hasFlag(msg.state, STATE::CONTROL::MUTE) ? MUTE_TIMEOUT : 0);
        
        // force node msg
        Timer<NODE_TIMER>::start(500ul + randomByte()/10);
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
  // setup PINS
  for (int i = 0; i < 4; i++) {
    digitalWrite(MOS_PINS[i], 1);
    pinMode(MOS_PINS[i], OUTPUT);
    analogWrite(MOS_PINS[i], 255);
  }

  digitalWrite(LED_PIN, HIGH);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);

  // setup and test LoRa
  radioInit = setupRadio();
}

void loop()
{
  static bool muted = false;
  static bool alarm = false;

  if (!radioInit)
  {
    flashFast();
  }
  else
  {      
    // process radio messages
    recvRadioMessages();
    
    // check node states
    muted = isMuted();
    alarm = isSirenOn();

    // switch siren output on [0] / off [255]
    if (alarm && muted)
    {
      analogWrite(MOS_ALARM_ALL_PIN, 0);
      analogWrite(MOS_ALARM_MUTED_PIN, 0);
      analogWrite(MOS_ALARM_NORMAL_PIN, 255);
    }
    else if (alarm)
    {
      analogWrite(MOS_ALARM_ALL_PIN, 0);
      analogWrite(MOS_ALARM_MUTED_PIN, 255);
      analogWrite(MOS_ALARM_NORMAL_PIN, 0);
    }
    else
    {
      analogWrite(MOS_ALARM_ALL_PIN, 255);
      analogWrite(MOS_ALARM_MUTED_PIN, 255);
      analogWrite(MOS_ALARM_NORMAL_PIN, 255);
    }

    // send out siren node messages
    if (Timer<NODE_TIMER>::hasExpired())
    {
      sendNodeMsg(false, false, false, muted);
      Timer<NODE_TIMER>::start(MSG_TIMEOUT - randomByte()*4ul);

      ssprintf(Serial, "tx: muted=%s\n", muted);
    }

    // show app status
    if (alarm)
    {
      flash();
    }
    else if (muted)
    {
      blinkAndFlash();
    }
    else
    {
      blink();
    }
  }
}
