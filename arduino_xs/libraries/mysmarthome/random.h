

// try too find a good random seed value
int getRandSeed()
{
  uint8_t seed = 0;
  for (int i = 0; i < 12; i++)
  {
    seed ^= (uint8_t)analogRead(i % 3);
  }
  
  return seed;
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
