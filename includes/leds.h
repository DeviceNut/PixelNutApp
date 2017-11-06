// PixelNutApp LED Control
// Uses single LED for error/status.
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if LED_ACTIVE_LOW
#define LED_ON    LOW
#define LED_OFF   HIGH
#else
#define LED_ON    HIGH
#define LED_OFF   LOW
#endif

void SetupLEDs(void)
{
  #if defined(DPIN_LED)
  pinMode(DPIN_LED, OUTPUT);
  digitalWrite(DPIN_LED, LED_ON);
  #endif
}

void BlinkStatusLED(uint16_t slow, uint16_t fast)
{
  #if defined(DPIN_LED)
  digitalWrite(DPIN_LED, LED_OFF);
  delay(250);

  if (slow)
  {
    for (int i = 0; i < slow; ++i)
    {
      digitalWrite(DPIN_LED, LED_ON);
      delay(750);
      digitalWrite(DPIN_LED, LED_OFF);
      delay(250);
    }
    if (fast) delay(250);
  }

  if (fast)
  {
    for (int i = 0; i < fast; ++i)
    {
      digitalWrite(DPIN_LED, LED_ON);
      delay(150);
      digitalWrite(DPIN_LED, LED_OFF);
      delay(250);
    }
    delay(250);
  }
  #endif
}

//========================================================================================
