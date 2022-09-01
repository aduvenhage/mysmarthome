
#include "battery.h"
#include "lora.h"
#include "led.h"
#include "random.h"


enum class MSG_APPLICATION : uint8_t
{
  SENSOR_XS = 0,
  SIREN,
  CONTROL
};

namespace MSG_STATE
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
    const uint8_t LOUD = 2;
    const uint8_t BTYLOW = 4;
    const uint8_t CHARGING = 8;
  };

  namespace CONTROL
  {
    const uint8_t MUTE = 1;
    const uint8_t LOUD = 2;
    const uint8_t BTYLOW = 4;
    const uint8_t CHARGING = 8;
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

