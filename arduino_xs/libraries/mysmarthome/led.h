
void switchLed(uint32_t period, uint8_t dutyCycle)
{
  static uint32_t t0 = 0;
  uint32_t t = millis();
  
  if (t - t0 < period)
  {
    if ((255 * (t - t0) / period) > (uint8_t)dutyCycle)
    {
      digitalWrite(LED_PIN, LOW);
    }
    else
    {
      digitalWrite(LED_PIN, HIGH);
    }
  }
  else
  {
    t0 = millis();
  }
}

void blink()
{
  switchLed(1000, 128);
}

void flash()
{
  switchLed(500, 192);
}

void flashFast()
{
  switchLed(200, 224);
}
