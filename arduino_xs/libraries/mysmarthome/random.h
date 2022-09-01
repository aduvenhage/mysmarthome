

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
