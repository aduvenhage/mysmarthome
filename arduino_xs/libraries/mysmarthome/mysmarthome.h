
#include "lora.h"
#include "led.h"

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


// try too find a good random seed value
int getRandSeed()
{
  for (int i = 0; i < 16; i++)
  {
    int seed = analogRead(0) + analogRead(1) + analogRead(2);
    if (seed > 0)
    {
        return seed;
    }
  }
  
  return 0;
}

// returns a random uint8
uint8_t randomByte()
{
  static bool seeded = false;
  if (!seeded)
  {
    randomSeed(getRandSeed());
  }

  return (uint8_t)random(256);
}

float getBatteryVoltage()
{
  // NOTE: multiplying by 2 since hardware has a devider on the pin
  static float vbatt = 0;
  vbatt = 0.9 * vbatt + 0.1 * analogRead(BTY_PIN) / BTY_VRR * REF_V / 1024.0;
  return vbatt;
}

float getBatteryPercentage()
{
  return (getBatteryVoltage() - BTY_MIN_V) / (BTY_MAX_V - BTY_MIN_V) * 100.0;
}

bool isBatteryLow()
{
  return getBatteryVoltage() < BTY_LOW_V;
}
