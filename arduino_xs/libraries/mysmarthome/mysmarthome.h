
#include "battery.h"
#include "lora.h"
#include "led.h"
#include "random.h"
#include "ssprintf.h"


enum class APPLICATION : uint8_t
{
  NODE = 0,
  CONTROL
};


namespace STATE
{
  enum class NODE : uint8_t
  {
    CLOSED   = 0b00000000,
    OPEN     = 0b00000001,
    BTYLOW   = 0b00000010,
    CHARGING = 0b00000100,
    MUTED    = 0b00001000
  };
  
  enum class CONTROL : uint8_t
  {
    NONE = 0b00000000,
    MUTE = 0b00000001
  };
  
  template <typename VALUE, typename FLAG>
  bool hasFlag(VALUE _value, FLAG _flag)
  {
    return (uint8_t)_value & (uint8_t)_flag;
  }
};


// radio message
#pragma pack(push)
#pragma pack(1)
struct Message
{
  uint32_t protoId = PROTO_ID;
  APPLICATION application = APPLICATION::NODE;
  uint32_t address = 0;
  uint8_t state = 0;
  uint8_t crc = 0;
};
#pragma pack(pop)


template <int ID>
struct Timer {
  static void start(unsigned long _timeout)
  {
    timeout = _timeout;
    timestamp = millis();
  }

  static bool hasExpired()
  {
    bool expired = (millis() - timestamp) >= timeout;
    if (expired)
    {
        timeout = 0;
    }
    
    return expired;
  }
  
  inline static unsigned long timeout = 0;
  inline static unsigned long timestamp = 0;
};


void sendNodeMsg(bool _open, bool _btyLow, bool _charging, bool _muted)
{
  Message msg;
  msg.address = ADDR;
  msg.application = APPLICATION::NODE;
  msg.state = _open ? (uint8_t)STATE::NODE::OPEN : 0;
  msg.state |= _btyLow ? (uint8_t)STATE::NODE::BTYLOW : 0;
  msg.state |= _charging ? (uint8_t)STATE::NODE::CHARGING : 0;
  msg.state |= _muted ? (uint8_t)STATE::NODE::MUTED : 0;

  sendRadioMsg(msg);
}


void sendCmdMsg(bool _mute)
{
  Message msg;
  msg.address = ADDR;
  msg.application = APPLICATION::CONTROL;
  msg.state = _mute ? (uint8_t)STATE::CONTROL::MUTE : 0;

  sendRadioMsg(msg);
}


template <>
struct Format<APPLICATION>
{
  static const char *go(const APPLICATION &_arg) {
    switch (_arg)
    {
      case APPLICATION::NODE:
        return "NODE";
        
      case APPLICATION::CONTROL:
        return "CONTROL";
        
      default:
        return "";
    }
  }
};


template <>
struct Format<STATE::NODE>
{
  static const char *go(const STATE::NODE &_arg) {
    if (STATE::hasFlag(_arg, STATE::NODE::OPEN)) return "OPEN";
    else if (STATE::hasFlag(_arg, STATE::NODE::BTYLOW)) return "BTYLOW";
    else if (STATE::hasFlag(_arg, STATE::NODE::MUTED)) return "MUTED";
    else return "CLOSED";
  }
};


template <>
struct Format<STATE::CONTROL>
{
  static const char *go(const STATE::CONTROL &_arg) {
    if (STATE::hasFlag(_arg, STATE::CONTROL::MUTE)) return "MUTE";
    else return "";
  }
};

