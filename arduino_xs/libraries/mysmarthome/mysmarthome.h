
#include "battery.h"
#include "lora.h"
#include "led.h"
#include "random.h"


enum class APPLICATION : uint8_t
{
  SENSOR_XS = 0,
  SIREN,
  CONTROL
};

namespace STATE
{
  namespace SENSOR_XS
  {
    const uint8_t OPEN = 1;
    const uint8_t BTYLOW = 2;
    const uint8_t CHARGING = 4;
  };
  
  namespace SIREN
  {
    const uint8_t ON = 1;
    const uint8_t BTYLOW = 2;
    const uint8_t CHARGING = 4;
  };

  namespace CONTROL
  {
    const uint8_t MUTE = 1;
    const uint8_t BTYLOW = 2;
    const uint8_t CHARGING = 4;
  };
};


// radio message
#pragma pack(push)
#pragma pack(1)
struct Message
{
  uint32_t protoId = PROTO_ID;
  uint32_t address = 0;
  uint8_t application = 0;
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
    return (millis() - timestamp) > timeout;
  }
  
  inline static unsigned long timeout = 0;
  inline static unsigned long timestamp = 0;
};

