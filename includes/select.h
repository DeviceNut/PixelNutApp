// PixelNutApp Pattern Selection Handling
// All pattern strings are stored in the code (eeprom flash).
//
// Sets global variables: 'cmdStr', 'curPattern', 'codePatterns'.
// Uses global variables: 'pPixelNutEngine', 'customPatterns[]'.
// Calls global routines: 'FlashGetStr', 'ErrorHandler'.
//========================================================================================
/*
Copyright (c) 2015-2020, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

void GetCurPattern(char *instr)
{
  #if CUSTOM_PATTERNS
  if ((curPattern > 0) && (curPattern <= codePatterns))
  {
    strcpy_P(instr, customPatterns[curPattern-1]);
    DBGOUT((F("Retrieved custom pattern #%d"), curPattern));
    pPixelNutEngine->popPluginStack(); // clear stack to prepare for new cmds
  }
  #endif
  #if EXTERN_PATTERNS
  #if CUSTOM_PATTERNS
  else
  #endif
  {
    // patterns are sent from external client and stored in flash
    // the pattern number is meaningful only to the client
    FlashGetStr(instr);
    DBGOUT((F("Retrieved external pattern #%d"), curPattern));
  }
  #endif
}

#if CUSTOM_PATTERNS

// never returns if fails to find any stored pattern strings
void CheckForPatterns(void)
{
  DBGOUT((F("Stored Patterns:")));

  codePatterns = 0;
  for (int i = 0; ; ++i)
  {
    if (customPatterns[i] == NULL)
    {
      codePatterns = i;
      break;
    }

    strcpy_P(cmdStr, customPatterns[i]);
    DBGOUT((F("  %2d: \"%s\""), i+1, cmdStr));
  }

  // cannot continue if cannot find any valid pattern strings
  if (!codePatterns) ErrorHandler(1, 1, true);
}

void GetNextPattern(void)
{
  // curPattern must be 1...codePatterns
  if (++curPattern > codePatterns)
    curPattern = 1;

  GetCurPattern(cmdStr);
}

void GetPrevPattern(void)
{
  // curPattern must be 1...codePatterns
  if (curPattern <= 1) curPattern = codePatterns;
  else --curPattern;

  GetCurPattern(cmdStr);
}

#else // !CUSTOM_PATTERNS

void CheckForPatterns(void) {}

#endif

//========================================================================================
