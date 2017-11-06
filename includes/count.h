// PixelNutApp Count Controls
// Sets pixel count property in the pixelnut engine from physical controls.
//
// Uses global variables: 'pixelNutSupport' and 'pixelCount'.
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if defined(DPIN_COUNT_BUTTON) || defined(APIN_COUNT_POT)

static void SetCountByPercent(byte percent)
{
  DBGOUT((F("Setting count property = %d%%"), percent));
  pPixelNutEngine->setCountProperty(percent);
}

#endif
//========================================================================================
#if defined(DPIN_COUNT_BUTTON)

// allow repeating and long press, but not double-click
UIDeviceButton bc_count(DPIN_COUNT_BUTTON, false, true);

static byte count_pos = 2; // default setting
static byte count_presets[] = { 0, 25, 50, 75, 100 }; // percentages

static void SetNewCount(void)
{
  if (count_pos >= sizeof(count_presets)/sizeof(count_presets[0]))
    count_pos = 0;

  SetCountByPercent( count_presets[count_pos] );
}

static void CheckCountButton(void)
{
  if (bc_count.CheckForChange() != UIDeviceButton::Retcode_NoChange)
  {
    ++count_pos;
    SetNewCount();
  }
}

static void SetupCountButton(void)
{
  // can adjust button settings here...

  SetNewCount();
}

//========================================================================================
// cannot use pixel count button AND a pot
#elif defined(APIN_COUNT_POT)

// allow headroom on output value to insure can reach min/max
UIDeviceAnalog pc_count(APIN_COUNT_POT, -10, MAX_PERCENTAGE+10);

static void CheckCountPot(void)
{
  if (pc_count.CheckForChange())
    SetCountByPercent( pixelNutSupport.clipValue(pc_count.newValue, 0, MAX_PERCENTAGE) );
}

static void SetupCountPot(void)
{
  SetCountByPercent(pc_count.newValue);
}

#endif
//========================================================================================

// initialize controls
void SetupCountControls(void)
{
  #if defined(DPIN_COUNT_BUTTON)
  SetupCountButton();
  #elif defined(APIN_COUNT_POT)
  SetupCountPot();
  #endif
}

// called every control loop
void CheckCountControls(void)
{
  #if defined(DPIN_COUNT_BUTTON)
  CheckCountButton();
  #elif defined(APIN_COUNT_POT)
  CheckCountPot();
  #endif
}

//========================================================================================
