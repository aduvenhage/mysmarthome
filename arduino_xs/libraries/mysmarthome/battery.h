
constexpr float VPIN_ALPHA = 0.02;
constexpr float VPIN_BETA = 1 - VPIN_ALPHA;

float getBatteryVoltage()
{
  // NOTE: multiplying by 2 since hardware has a devider on the pin
  static float vbatt = 0;
  vbatt = VPIN_BETA * vbatt + VPIN_ALPHA * analogRead(BTY_PIN) / BTY_VRR * REF_V / 1024.0;
  return vbatt;
}

float getBatteryPercentage()
{
  float p = (getBatteryVoltage() - BTY_MIN_V) / (BTY_MAX_V - BTY_MIN_V) * 100;
  if (p > 100.0) return 100.0;
  else if (p < 1.0) return 1.0;
  return p;
}

bool isBatteryLow()
{
  return getBatteryVoltage() < BTY_LOW_V;
}

float getChargeVoltage()
{
  // NOTE: multiplying by 2 since hardware has a devider on the pin
  static float vchg = 0;
  vchg = VPIN_BETA * vchg + VPIN_ALPHA * analogRead(CHG_PIN) / CHG_VRR * REF_V / 1024.0;
  return vchg;
}

bool isBatteryCharging()
{
  return getChargeVoltage() > getBatteryVoltage();
}
