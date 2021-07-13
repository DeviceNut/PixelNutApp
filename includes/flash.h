// PixelNutApp Flash Storage Handling
// Stores the pattern number, effect properties, and pattern strings in (eeprom flash).
//
// Uses global variables: 'pPixelNutEngine'.
// Set global variable: 'curPattern'.
//========================================================================================
/*
Copyright (c) 2015-2020, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if !defined(EEPROM_BYTES)
#if defined(__arm__) && defined(__MKL26Z64__)         // Teensy LC
#define EEPROM_BYTES            128
#elif defined(__arm__) && defined(__MK20DX256__)      // Teensy 3.2
#define EEPROM_BYTES            2048
#elif defined(__AVR__)                                // all other AVR processors
#define EEPROM_BYTES            1024
#elif defined(ESP32)                                  // Espressif ESP32
#define EEPROM_BYTES            4096
#else                                                 // disable for unknown processor
#define EEPROM_BYTES            0
#warning("No EEPROM - unspecified processor")
#endif
#endif

#if (EEPROM_BYTES <= 0)
#define EEPROM_FREE_BYTES 0
#define FLASHOFF_PATTERN_START 0
#define FLASHOFF_PATTERN_END 0
void FlashSetStr(char *str, int offset) {}
void FlashGetStr(char *str) { *str = 0; }
void FlashSetBright() {}
void FlashSetDelay()  {}
void FlashSetPattern(byte pattern)  {}
void FlashSetXmode(bool enable) {}
void FlashSetExterns(uint16_t hue, byte wht, byte cnt) {}
void FlashSetForce(short force) {}
short FlashGetForce(void) { return 0; }
void FlashSetProperties(void) {}
void FlashStartup(void) {}

#if EXTERNAL_COMM
#error("Must have EEPROM to use external communications")
#endif

#else // (EEPROM_BYTES > 0)

#include <EEPROM.h>

// for each segment:
#define FLASH_SEG_LENGTH        13

// offsets within each segment space:
#define FLASH_SEG_BRIGHTNESS    0
#define FLASH_SEG_DELAYMSECS    1
#define FLASH_SEG_FIRSTPOS      2  // 2 bytes
#define FLASH_SEG_DIRECTION     4
#define FLASH_SEG_PATTERN       5
#define FLASH_SEG_XT_MODE       6
#define FLASH_SEG_XT_HUE        7  // 2 bytes
#define FLASH_SEG_XT_WHT        9
#define FLASH_SEG_XT_CNT        10
#define FLASH_SEG_FORCE         11 // 2 bytes

#define FLASHOFF_DEVICE_NAME    0
#if EXTERNAL_COMM
#define FLASHOFF_SEGMENT_DATA   MAXLEN_DEVICE_NAME
#else
#define FLASHOFF_SEGMENT_DATA   FLASHOFF_DEVICE_NAME
#endif

#if EXTERN_PATTERNS
#define FLASHLEN_PATTERN        STRLEN_PATTERNS
#else
#define FLASHLEN_PATTERN        0
#endif

#if STRANDS_MULTI
#define FLASHOFF_PATTERN_START  (FLASHOFF_SEGMENT_DATA  + (SEGMENT_COUNT * FLASH_SEG_LENGTH))
#define FLASHOFF_PATTERN_END    (FLASHOFF_PATTERN_START + (SEGMENT_COUNT * FLASHLEN_PATTERN))
#else // logical segments use a single pattern string
#define FLASHOFF_PATTERN_START  (FLASHOFF_SEGMENT_DATA + (SEGMENT_COUNT * FLASH_SEG_LENGTH))
#define FLASHOFF_PATTERN_END    (FLASHOFF_PATTERN_START + FLASHLEN_PATTERN)
#endif

#if (FLASHOFF_PATTERN_END > EEPROM_BYTES)
#error("Not enough flash space to store external pattern strings");
#endif

#define EEPROM_FREE_START  FLASHOFF_PATTERN_END
#define EEPROM_FREE_BYTES  (EEPROM_BYTES - EEPROM_FREE_START)

static byte valOffset = FLASHOFF_SEGMENT_DATA;
static uint16_t strOffset = FLASHOFF_PATTERN_START;

static void FlashStart(void)
{
#if defined(ESP32)
  EEPROM.begin(EEPROM_BYTES);
#endif
}

static void FlashDone(void)
{
#if defined(ESP32)
  EEPROM.commit();
#endif
}

static void FlashSetValue(uint16_t offset, byte value) { EEPROM.write(valOffset + offset, value); }
       byte FlashGetValue(uint16_t offset) { return EEPROM.read(valOffset + offset); }

// Note: this is not range checked
void FlashSetSegment(byte segindex)
{
  valOffset = FLASHOFF_SEGMENT_DATA + (segindex * FLASH_SEG_LENGTH);

  #if STRANDS_MULTI // logical segments use a single pattern string
  strOffset = FLASHOFF_PATTERN_START + (segindex * FLASHLEN_PATTERN);
  #endif
}

void FlashSetName(char *name)
{
  DBGOUT((F("FlashSetName: \"%s\""), name));

  for (int i = 0; i < MAXLEN_DEVICE_NAME; ++i)
    EEPROM.write((FLASHOFF_DEVICE_NAME + i), name[i]);

  FlashDone();
}

void FlashGetName(char *name)
{
  for (int i = 0; i < MAXLEN_DEVICE_NAME; ++i)
    name[i] = EEPROM.read(FLASHOFF_DEVICE_NAME + i);

  name[MAXLEN_DEVICE_NAME] = 0;

  DBGOUT((F("FlashGetName: \"%s\""), name));
}

void FlashSetStr(char *str, int offset)
{
  DBGOUT((F("FlashSetStr(@%d): \"%s\" (len=%d)"), (strOffset + offset), str, strlen(str)));

  for (int i = 0; ; ++i)
  {
    if ((strOffset + offset + i) >= EEPROM_BYTES) break; // prevent overrun
    EEPROM.write((strOffset + offset + i), str[i]);
    if (!str[i]) break;
  }

  FlashDone();
}

void FlashGetStr(char *str)
{
  for (int i = 0; ; ++i)
  {
    if ((i >= (STRLEN_PATTERNS-1)) ||
        ((strOffset + i) >= EEPROM_BYTES))
         str[i] = 0; // prevent overrun
    else str[i] = EEPROM.read(strOffset + i);
    if (!str[i]) break;
  }

  DBGOUT((F("FlashGetStr(@%d): \"%s\" (len=%d)"), strOffset, str, strlen(str)));
}

void FlashSetBright()    { FlashSetValue(FLASH_SEG_BRIGHTNESS, pPixelNutEngine->getMaxBrightness());  FlashDone(); }
void FlashSetDelay()     { FlashSetValue(FLASH_SEG_DELAYMSECS, pPixelNutEngine->getDelayOffset());    FlashDone(); }
void FlashSetFirst()     { FlashSetValue(FLASH_SEG_FIRSTPOS,   pPixelNutEngine->getFirstPosition());  FlashDone(); }
void FlashSetDirection() { FlashSetValue(FLASH_SEG_DIRECTION,  pPixelNutEngine->getDirection());      FlashDone(); }

void FlashSetPattern(byte pattern)  { FlashSetValue(FLASH_SEG_PATTERN, pattern); FlashDone(); }
void FlashSetXmode(bool enable)     { FlashSetValue(FLASH_SEG_XT_MODE, enable);  FlashDone(); }

void FlashSetExterns(uint16_t hue, byte wht, byte cnt)
{
  FlashSetValue(FLASH_SEG_XT_HUE,   hue&0xFF);
  FlashSetValue(FLASH_SEG_XT_HUE+1, hue>>8);
  FlashSetValue(FLASH_SEG_XT_WHT,   wht);
  FlashSetValue(FLASH_SEG_XT_CNT,   cnt);
  FlashDone();
}

void FlashSetForce(short force)
{
  FlashSetValue(FLASH_SEG_FORCE, force);
  FlashSetValue(FLASH_SEG_FORCE+1, (force >> 8));
  FlashDone();
}

short FlashGetForce(void)
{
  return FlashGetValue(FLASH_SEG_FORCE) + (FlashGetValue(FLASH_SEG_FORCE+1) << 8);
}

void FlashSetProperties(void)
{
  uint16_t hue = FlashGetValue(FLASH_SEG_XT_HUE) + (FlashGetValue(FLASH_SEG_XT_HUE+1) << 8);

  pPixelNutEngine->setPropertyMode(      FlashGetValue(FLASH_SEG_XT_MODE));
  pPixelNutEngine->setColorProperty(hue, FlashGetValue(FLASH_SEG_XT_WHT));
  pPixelNutEngine->setCountProperty(     FlashGetValue(FLASH_SEG_XT_CNT));
}

void FlashStartup(void)
{
  FlashStart();

  curPattern = FlashGetValue(FLASH_SEG_PATTERN);
  if (!curPattern) curPattern = 1; // starts with 1
  #if !EXTERN_PATTERNS
  if (curPattern > codePatterns) curPattern = 1;
  #endif
 
  DBGOUT((F("Flash: pattern=#%d"), curPattern));

  byte bright = FlashGetValue(FLASH_SEG_BRIGHTNESS);
  if (bright == 0) FlashSetValue(FLASH_SEG_BRIGHTNESS, bright=MAX_BRIGHTNESS); // set to max if 0

  int8_t delay = (int8_t)FlashGetValue(FLASH_SEG_DELAYMSECS);
  if ((delay < -DELAY_RANGE) || (DELAY_RANGE < delay)) delay = 0; // set to min if out of range

  int16_t fpos = FlashGetValue(FLASH_SEG_FIRSTPOS);

  pPixelNutEngine->setMaxBrightness(bright);
  pPixelNutEngine->setDelayOffset(delay);
  pPixelNutEngine->setFirstPosition(fpos);

  DBGOUT((F("Flash: brightness=%d%%"), bright));
  DBGOUT((F("Flash: delay=%d msecs"), (int8_t)FlashGetValue(FLASH_SEG_DELAYMSECS)));

  FlashSetProperties();

  FlashDone();
}

#if EEPROM_FORMAT
void FlashFormat(void)
{
  FlashStart();
  for (int i = 0; i < EEPROM_BYTES; ++i) EEPROM.write(i, 0);
  FlashDone();

  DBGOUT((F("Cleared %d bytes of EEPROM"), EEPROM_BYTES));
}
#endif

#endif // (EEPROM_BYTES > 0)

//========================================================================================
