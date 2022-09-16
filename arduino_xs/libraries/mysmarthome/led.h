
void switchLed(uint32_t period, uint8_t dutyCycle)
{
  static uint32_t t0 = 0;
  uint32_t t = millis();
  
  if (t - t0 < period)
  {
    if ((255 * (t - t0) / period) > (uint8_t)dutyCycle)
    {
      digitalWrite(LED_PIN, !LED_ON);
    }
    else
    {
      digitalWrite(LED_PIN, LED_ON);
    }
  }
  else
  {
    t0 = millis();
  }
}

void blink()
{
  switchLed(2000, 128);
}

void flash()
{
  switchLed(500, 64);
}

void flashFast()
{
  switchLed(100, 64);
}

void blinkAndFlash()
{
  (millis() / 3000) % 2 ? blink() : flash();
}

void blinkAndFlashFast()
{
  (millis() / 3000) % 2 ? blink() : flashFast();
}
