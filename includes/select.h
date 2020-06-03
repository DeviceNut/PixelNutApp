// PixelNutApp Pattern Selection Handling
// All pattern strings are stored in the code (eeprom flash).
//
// Sets global variables: 'cmdStr', 'curPattern', 'codePatterns'.
// Uses global variables: 'pPixelNutEngine', 'customPatterns[]'.
// Calls global routines: 'FlashGetStr', 'ErrorHandler'.
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/
#if CUSTOM_PATTERNS

// returns false if fails to find any stored pattern strings
void CheckForPatterns(void)
{
  DBGOUT((F("Read patterns from flash code")));

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

  DBGOUT((F("Number of code patterns = %d"), codePatterns));

  // cannot continue if cannot find any valid pattern strings
  if (!codePatterns) ErrorHandler(1, 1, true);

  curPattern = FlashGetPattern();

  #if !EXTERN_PATTERNS // else client handles it
  if (!curPattern || (curPattern > codePatterns))
    curPattern = 1;
  #endif

  DBGOUT((F("Starting pattern = #%d"), curPattern));
}

void GetCurPattern(char *instr)
{
  // check for an internal pattern first
  if ((curPattern > 0) && (curPattern <= codePatterns))
  {
    DBGOUT((F("Copying pattern = #%d"), curPattern));
    strcpy_P(instr, customPatterns[curPattern-1]);
  }
  #if EXTERN_PATTERNS
  // else retrieve pattern from flash for current segment
  else FlashGetStr(instr);
  #else
  else instr[0] = 0;
  #endif

  if (instr[0])
  {
    DBGOUT((F("Retrieved pattern %d (len=%d)"), curPattern, strlen(instr)));
    pPixelNutEngine->popPluginStack(); // clear stack to prepare for new cmds
  }
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
  // curPattern must be 0...codePatterns
  if (curPattern <= 1) curPattern = codePatterns;
  else --curPattern;

  GetCurPattern(cmdStr);
}

#else // !CUSTOM_PATTERNS

void CheckForPatterns(void)
{
  curPattern = FlashGetPattern();
}

void GetCurPattern(char *instr)
{
  FlashGetStr(instr);
  DBGOUT((F("Retrieved pattern %d (len=%d)"), curPattern, strlen(instr)));
}

#endif // CUSTOM_PATTERNS
//========================================================================================
