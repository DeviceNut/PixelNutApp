// PixelNutApp Debug and Error Handling Routines
//
// Uses global variables: 'pixelNutSupport'.
// Calls global functions: 'BlinkStatusLED()'.
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if DEBUG_OUTPUT
#define DBG(x) x
#define DBGOUT(x) MsgFormat x

// debug output string must be longer than pattern strings and any debug format string
#define MAXLEN_FMTSTR (STRLEN_PATTERNS + 20)
#define MAXLEN_DBGSTR (STRLEN_PATTERNS + 70)

char fmtstr[MAXLEN_FMTSTR];   // holds debug format string
char outstr[MAXLEN_DBGSTR];   // holds debug output string

void MsgFormat(const __FlashStringHelper *str_in_code, ...)
{
  strcpy_P(fmtstr, (char *)str_in_code);

  va_list va;
  va_start(va, str_in_code);
  vsnprintf(outstr, MAXLEN_DBGSTR, fmtstr, va);
  va_end(va);

  Serial.println(outstr);
}

void SetupDebugInterface(void)
{
  Serial.begin(115200); // MATCH THIS BAUD RATE <<<

  // wait up to 5 secs for serial window
  uint32_t tout = millis() + 5000;
  while (!Serial) if (millis() > tout) break;

  // function called from DBGOUT macro
  pixelNutSupport.msgFormat = MsgFormat;

  // now can send sign-on message
  DBGOUT((F(DEBUG_SIGNON)));
}

#else
#define DBG(x)
#define DBGOUT(x)
void SetupDebugInterface(void) {}
#endif

// does not return if 'dostop' is true
void ErrorHandler(short slow, short fast, bool dostop)
{
  DBGOUT((F("Error code: 0x%02X"), ((slow << 4) | fast)));

  do BlinkStatusLED(slow, fast);
  while (dostop);
}
