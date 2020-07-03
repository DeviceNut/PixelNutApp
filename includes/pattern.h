// PixelNutApp Pattern Selection Controls
// Selects the current preset pattern from physical controls.
//
// Uses global variables: 'pPixelNutEngine'.
// Calls global functions: 'GetPrevPattern()', 'GetNextPattern()', 'FlashSetPattern'.
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if defined(DPIN_PATTERN_BUTTON)

// use double-click for triggering the pattern only if not separate button
#if defined(DPIN_TRIGGER_BUTTON)
#define DO_DOUBLE_CLICK false
#else
#define DO_DOUBLE_CLICK true
#endif

// no repeating allowed, but use single/long press for pattern change/save
UIDeviceButton bc_pattern(DPIN_PATTERN_BUTTON, DO_DOUBLE_CLICK, true);

#if defined(DPIN_PAT_BUTTON_PREV)
// no repeating allowed, but use single/long press for pattern change/save
UIDeviceButton bc_pat_prev(DPIN_PAT_BUTTON_PREV, DO_DOUBLE_CLICK, true);
#endif

static void CheckPatternButton(void)
{
  switch ( bc_pattern.CheckForChange() )
  {
    case UIDeviceButton::Retcode_Single:
    {
      DBGOUT((F("Pattern button: Single (next pattern)")));
      GetNextPattern();
      break;
    }
    #if !defined(DPIN_TRIGGER_BUTTON)
    case UIDeviceButton::Retcode_Double:
    {
      short force;
      uint16_t msecs = bc_pattern.msecsPressed;
      // 1 second press == MAX_FORCE_VALUE
      if (msecs > MAX_FORCE_VALUE)
           force = MAX_FORCE_VALUE;
      else force = msecs;

      DBGOUT((F("Pattern button: Double (force=%d)"), force));
      pPixelNutEngine->triggerForce(force);
      break;
    }
    #endif
    case UIDeviceButton::Retcode_LPress:
    {
      while (digitalRead(DPIN_PATTERN_BUTTON) == LOW); // wait for release
      DBGOUT((F("Pattern button: LPress (save pattern #%d)"), curPattern));
      FlashSetPattern(curPattern);
      break;
    }
    default: break;
  }

  #if defined(DPIN_PAT_BUTTON_PREV)

  switch ( bc_pat_prev.CheckForChange() )
  {
    case UIDeviceButton::Retcode_Single:
    {
      DBGOUT((F("Pattern button: Single (prev pattern)")));
      GetPrevPattern();
      break;
    }
    #if !defined(DPIN_TRIGGER_BUTTON)
    case UIDeviceButton::Retcode_Double:
    {
      short force;
      uint16_t msecs = bc_pat_prev.msecsPressed;
      // 1 second press == MAX_FORCE_VALUE
      if (msecs > MAX_FORCE_VALUE)
           force = MAX_FORCE_VALUE;
      else force = msecs;

      DBGOUT((F("Pattern button: Double (force=%d)"), force));
      pPixelNutEngine->triggerForce(force);
      break;
    }
    #endif
    case UIDeviceButton::Retcode_LPress:
    {
      while (digitalRead(DPIN_PAT_BUTTON_PREV) == LOW); // wait for release
      DBGOUT((F("Pattern button: LPress (save pattern #%d)"), curPattern));
      FlashSetPattern(curPattern);
      break;
    }
    default: break;
  }
  #endif
}

static void SetupPatternButton(void)
{
  // can adjust button settings here...
}
#endif

//========================================================================================

// initialize controls
void SetupPatternControls(void)
{
  #if defined(DPIN_PATTERN_BUTTON)
  SetupPatternButton();
  #endif
}

// called every control loop
void CheckPatternControls(void)
{
  #if defined(DPIN_PATTERN_BUTTON)
  CheckPatternButton();
  #endif
}

//========================================================================================
