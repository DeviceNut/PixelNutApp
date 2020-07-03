// PixelNutApp Debug and Error Handling Routines
//
// Uses global variables: 'pixelNutSupport'.
// Calls global functions: 'BlinkStatusLED()'.
//========================================================================================
/*
Copyright (c) 2015-2020, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if DEBUG_OUTPUT
#define DBG(x) x
#define DBGOUT(x) MsgFormat x

#if defined(SPARK) || defined(ESP32)
#undef F
#define F(x) x
#endif

// debug output string must be longer than pattern strings and any debug format string
#define MAXLEN_FMTSTR (STRLEN_PATTERNS + 20)
#define MAXLEN_DBGSTR (STRLEN_PATTERNS + 70)

char fmtstr[MAXLEN_FMTSTR];   // holds debug format string
char outstr[MAXLEN_DBGSTR];   // holds debug output string

#if defined(SPARK) || defined(ESP32)
void MsgFormat(const char *fmtstr, ...)
{
  va_list va;
  va_start(va, fmtstr);
  vsnprintf(outstr, MAXLEN_DBGSTR, fmtstr, va);
  va_end(va);

  Serial.println(outstr);
}
#else
void MsgFormat(const __FlashStringHelper *str_in_code, ...)
{
  strcpy_P(fmtstr, (char*)str_in_code);

  va_list va;
  va_start(va, str_in_code);
  vsnprintf(outstr, MAXLEN_DBGSTR, fmtstr, va);
  va_end(va);

  Serial.println(outstr);
}
#endif

void SetupDBG(void)
{
  // function called from DBGOUT macro
  pixelNutSupport.msgFormat = MsgFormat;

  Serial.begin(115200); // MATCH THIS BAUD RATE <<<

  #if defined(SPARK)
  // wait up to 15 secs for serial window
  uint32_t tout = millis() + 15000;
  // on Windows only: user should have serial terminal closed first,
  // then start running this, and then open terminal and press a key
  while (!Serial.available())
  {
    if (millis() > tout) break;
    BlinkStatusLED(0, 1);
  }

  #elif defined(ESP32)
  // wait up to 5 secs for something to be sent from the serial window
  uint32_t tout = millis() + 5000;
  while (!Serial.available()) if (millis() > tout) break;

  #else // !SPARK
  // wait up to 5 secs for serial window
  uint32_t tout = millis() + 5000;
  while (!Serial) if (millis() > tout) break;
  #endif

  // wait a tad longer, then send sign-on message
  delay(10);
  DBGOUT((F(DEBUG_SIGNON)));
}

#else
#define DBG(x)
#define DBGOUT(x)
void SetupDBG(void) {}
#endif

// does not return if 'dostop' is true
void ErrorHandler(short slow, short fast, bool dostop)
{
  DBGOUT((F("Error code: 0x%02X"), ((slow << 4) | fast)));

  do BlinkStatusLED(slow, fast);
  while (dostop);
}
