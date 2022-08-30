
struct Sensor
{
  const char *name;
  uint32_t address;
  uint32_t timestamp;
  uint8_t state;
};

Sensor sensors[] = {
  {"HEKMOTOR", 'HEKM', 0, 0},
  {"AFDAK BEAM", 'AFDK', 0, 0},
  {"VOOR BEAM", 'VOOR', 0, 0},
};

constexpr int NUM_SENSORS = sizeof(sensors) / sizeof(Sensor); 
constexpr uint32_t PROTO_ID = 'MSHM';
constexpr uint32_t ADDR = 'DISP';
constexpr float BTY_LOW_V = 3.6;
