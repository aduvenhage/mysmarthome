
struct Node
{
  const char *name;
  uint32_t address;
  uint32_t timestamp;
  uint8_t state;
};

Node nodes[] = {
  {"HEKMOTOR", 'HEKM', 0, 0},
  {"AFDAK BEAM", 'AFDK', 0, 0},
  {"VOOR BEAM", 'VOOR', 0, 0},
  {"SIREN", 'SIRN', 0, 0},
};

constexpr uint8_t NUM_NODES = sizeof(nodes) / sizeof(Node); 
constexpr uint32_t PROTO_ID = 'MSHM';
constexpr uint32_t ADDR = 'DISP';
