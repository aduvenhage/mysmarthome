
struct Sensor
{
  const char *name;
  uint32_t address;
  uint32_t timestamp;
  uint8_t state;
};

Sensor sensors[] = {
  {"HEK", 1, 0, 0},
  {"AFDAK", 2, 0, 0},
  {"VOOR", 3, 0, 0},
};

constexpr int NUM_SENSORS = sizeof(sensors) / sizeof(Sensor); 
