
#include "lora.h"
#include "led.h"

const uint32_t PROTO_ID = 'MSHS';
const uint8_t SENSOR_STATE_OPEN = 1;
const uint8_t SENSOR_STATE_BTYLOW = 2;
const uint8_t SENSOR_STATE_ERROR = 4;

enum class EApplication : uint8_t
{
    Open = 0,
    LoraBase,
};

#pragma pack(push)
#pragma pack(1)
struct Message
{
  uint32_t protoId = PROTO_ID;
  uint32_t address = 0;
  EApplication application = EApplication::Open;
  uint8_t state = 0;
  uint8_t crc = 0;
};
#pragma pack(pop)
