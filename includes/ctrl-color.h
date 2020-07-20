// PixelNutApp Color Controls
// Sets color hue/white properties in pixelnut engine from physical controls.
//
// Uses global variables: 'pPixelNutEngine'.
//========================================================================================
/*
Copyright (c) 2015-2020, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if defined(APIN_HUE_POT) && defined(APIN_WHITE_POT)

UIDeviceAnalog pc_hue(APIN_HUE_POT, 0, MAX_DEGREES_HUE);
UIDeviceAnalog pc_white(APIN_WHITE_POT, 0, MAX_PERCENTAGE);

static void SetColorProp(void)
{
  uint16_t hue_val = pc_hue.newValue;
  byte white_val = pc_white.newValue;
  DBGOUT((F("Setting hue = %d degrees and white = %d%%"), hue_val, white_val));
  pPixelNutEngine->setColorProperty(hue_val, white_val);
}

static void CheckColorPots(void)
{
  if (pc_hue.CheckForChange() || pc_white.CheckForChange())
    SetColorProp();
}
#endif

//========================================================================================

// initialize controls
void SetupColorControls(void)
{
  #if defined(APIN_HUE_POT) && defined(APIN_WHITE_POT)
  SetColorProp();
  #endif
}

// called every control loop
void CheckColorControls(void)
{
  #if defined(APIN_HUE_POT) && defined(APIN_WHITE_POT)
  CheckColorPots();
  #endif
}

//========================================================================================
