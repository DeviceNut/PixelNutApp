// PixelNutApp Brightness Controls
// Sets global max brightness applied to all effects in the pixelnut engine.
//
// Uses global variables: 'pPixelNutEngine', 'pixelNutSupport'.
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if defined(DPIN_BRIGHT_BUTTON) || defined(APIN_BRIGHT_POT)

// engine support setting of global maximum brightness
static void SetMaxBrightness(byte brightness)
{
  DBGOUT((F("Controls: max brightness=%d%%"), brightness));
  pPixelNutEngine->setMaxBrightness(brightness);
}

#endif

//========================================================================================
#if defined(DPIN_BRIGHT_BUTTON)

// allow repeating and long press, but not double-click
UIDeviceButton bc_bright(DPIN_BRIGHT_BUTTON, false, true);

static byte bright_pos = 2; // default setting
static byte bright_presets[] = { 40, 55, 70, 85, 100 }; // percentages

static void SetNewBrightness(void)
{
  if (bright_pos >= sizeof(bright_presets)/sizeof(bright_presets[0]))
    bright_pos = 0;

  // use maximum brightness value to attenuate new value
  uint16_t newval = (bright_presets[bright_pos] * MAX_BRIGHTNESS) / MAX_PERCENTAGE;
  SetMaxBrightness(newval);
}

static void CheckBrightness(void)
{
  if (bc_bright.CheckForChange() != UIDeviceButton::Retcode_NoChange)
  {
    ++bright_pos;
    SetNewBrightness();
  }
}

void SetupBrightness(void)
{
  // can adjust button settings here...

  SetNewBrightness();
}

//========================================================================================
// cannot use brightness button AND a pot
#elif defined(APIN_BRIGHT_POT)

#if BRIGHT_POT_BACKWARDS // if wired the pot backwards
UIDeviceAnalog pc_bright(APIN_BRIGHT_POT, MAX_PERCENTAGE, 0);
#else
// allow headroom on output value to insure can reach max
UIDeviceAnalog pc_bright(APIN_BRIGHT_POT, 0, MAX_PERCENTAGE+10);
#endif

static void SetNewBrightness(void)
{
  // use maximum brightness value to attenuate new value
  long bval = pixelNutSupport.clipValue(pc_bright.newValue, 0, 100);
  bval = (bval * MAX_BRIGHTNESS) / MAX_PERCENTAGE;
  SetMaxBrightness((byte)bval);
}

static void CheckBrightness(void)
{
  if (pc_bright.CheckForChange())
    SetNewBrightness();
}

static void SetupBrightness(void)
{
  SetNewBrightness();
}

#endif
//========================================================================================

// initialize controls
void SetupBrightControls(void)
{
  #if defined(DPIN_BRIGHT_BUTTON) || defined(APIN_BRIGHT_POT)
  SetupBrightness();
  #endif
}

// called every control loop
void CheckBrightControls(void)
{
  #if defined(DPIN_BRIGHT_BUTTON) || defined(APIN_BRIGHT_POT)
  CheckBrightness();
  #endif
}

//========================================================================================
