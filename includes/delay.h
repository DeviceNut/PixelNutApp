// PixelNutApp Delay Control
// Sets global delay offset applied to all effects in the pixelnut engine.
//
// Uses global variables: 'pPixelNutEngine'.
//========================================================================================
/*
Copyright (c) 2015-2020, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if defined(DPIN_DELAY_BUTTON) || defined(APIN_DELAY_POT)

// engine support setting of global delay offset
static void SetDelayOffset(int8_t msecs = 0)
{
  DBGOUT((F("Controls: delay offset=%d msecs"), (msecs + DELAY_OFFSET)));
  pPixelNutEngine->setDelayOffset(msecs + DELAY_OFFSET);
}

#endif

//========================================================================================
#if defined(DPIN_DELAY_BUTTON)

// allow repeating and long press, but not double-click
UIDeviceButton bc_delay(DPIN_DELAY_BUTTON, false, true, true);

static byte delay_pos = 2; // default setting
static int8_t delay_presets[] = { 30, 15, 0, -15, -30 };

static void SetNewDelay(void)
{
  if (delay_pos >= sizeof(delay_presets)/sizeof(delay_presets[0]))
    delay_pos = 0;

  SetDelayOffset( delay_presets[delay_pos] );
}

static void CheckDelay(void)
{
  if (bc_delay.CheckForChange() != UIDeviceButton::Retcode_NoChange)
  {
    ++delay_pos;
    SetNewDelay();
  }
}

static void SetupDelay(void)
{
  // can adjust button settings here...

  SetNewDelay();
}

//========================================================================================
// cannot use delay button AND a pot
#elif defined(APIN_DELAY_POT)

// we define this so that the animation gets faster when increasing the pot
#if defined(DELAY_POT_BACKWARDS) && DELAY_POT_BACKWARDS
UIDeviceAnalog pc_delay(APIN_DELAY_POT, -DELAY_RANGE, DELAY_RANGE);
#else // default is not wired backwards
UIDeviceAnalog pc_delay(APIN_DELAY_POT, DELAY_RANGE, -DELAY_RANGE);
#endif

static void CheckDelay(void)
{
  if (pc_delay.CheckForChange())
    SetDelayOffset( pc_delay.newValue );
}

static void SetupDelay(void)
{
  DBGOUT((F("Delay: range=%d offset=%d msecs"), DELAY_RANGE, DELAY_OFFSET));
  SetDelayOffset( pc_delay.newValue );
}

#endif
//========================================================================================

// initialize controls
void SetupDelayControls(void)
{
  #if defined(DPIN_DELAY_BUTTON) || defined(APIN_DELAY_POT)
  SetupDelay();
  #endif
}

// called every control loop
void CheckDelayControls(void)
{
  #if defined(DPIN_DELAY_BUTTON) || defined(APIN_DELAY_POT)
  CheckDelay();
  #endif
}

//========================================================================================
