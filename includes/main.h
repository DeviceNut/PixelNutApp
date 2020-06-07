// PixelNutApp Main Setup and Loop Routines
//
// Uses global variables: 'pPixelNutEngine', 'pCustomCode', 'pNeoPixels',
//                        'pPixelBytes', 'doUpdate'.
//
// Calls global functions: 'SetupLEDs', 'SetupDebugInterface', 'ErrorHandler',
//                         'BlinkStatusLED', 'FlashFormat', 'FlashStartup',
//                         'CheckForPatterns', 'GetCurPattern', and all
//                         'Setup<>Controls' and 'Check<>Controls' calls.
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if !MAIN_OVERRIDE

#if !SHOWPIX_OVERRIDE
void ShowPixels(void)
{
  #if PIXELS_APA
  byte *ptr = pPixelData;

  SPI.beginTransaction(spiSettings);

  // 4 byte start-frame marker
  for (int i = 0; i < 4; i++) SPI.transfer(0x00);

  for (int i = 0; i < PIXEL_COUNT; ++i)
  {
    SPI.transfer(0xFF);
    for (int j = 0; j < 3; j++) SPI.transfer(*ptr++);
  }

  SPI.endTransaction();
  #else
  pNeoPixels->show(pPixelData, (PIXEL_COUNT*3));
  #endif
}
#else
extern void ShowPixels(void);
#endif

void CheckExecCmd(char *instr)
{
  if (instr[0]) // if have new command for engine
  {
    DBGOUT((F("Exec: \"%s\""), instr));

    PixelNutEngine::Status status = pPixelNutEngine->execCmdStr(instr);
    if (status != PixelNutEngine::Status_Success)
    {
      DBGOUT((F("CmdErr: %d"), status));
      ErrorHandler(2, status, false); // blink for error and continue
    }
    else pCustomCode->pattern();

    instr[0] = 0; // must clear command string after finished
  }
}

void setup()
{
  SetupLEDs(); // status LED: indicate in setup now
  SetupDebugInterface(); // setup/wait for debug monitor
  // Note: cannot use debug output until above is used,
  // meaning DBGOUT() cannot be used in constructors.

  // set seed to value read from unconnected analog port
  randomSeed(analogRead(APIN_SEED));

  #if (PIXELS_APA && !BLUEFRUIT_BLE)
  SPI.begin(); // initialize SPI library
  #endif

  // turn off all pixels
  memset(pPixelData, 0, (PIXEL_COUNT*3));
  ShowPixels();

  #if EEPROM_FORMAT
  FlashFormat(); // format entire EEPROM
  pCustomCode->flash(); // custom flash handling
  ErrorHandler(0, 3, true);
  #endif

  if (pPixelNutEngine->pDrawPixels == NULL)
  {
    DBGOUT((F("Failed to alloc pixel engine buffers")));
    ErrorHandler(2, PixelNutEngine::Status_Error_Memory, true);
  }

  #if EXTERNAL_COMM
  FlashStartup(); // get saved settings from flash
  #endif

  DBGOUT((F("Configuration Settings:")));
  #if !EXTERNAL_COMM
  DBGOUT((F("  MAX_BRIGHTNESS    = %d"), MAX_BRIGHTNESS));
  DBGOUT((F("  DELAY_OFFSET      = %d"), DELAY_OFFSET));
  #endif
  DBGOUT((F("  PIXEL_COUNT       = %d"), PIXEL_COUNT));
  DBGOUT((F("  STRAND_COUNT      = %d"), STRAND_COUNT));
  DBGOUT((F("  SEGMENT_COUNT     = %d"), SEGMENT_COUNT));
  DBGOUT((F("  STRLEN_PATTERNS   = %d"), STRLEN_PATTERNS));
  DBGOUT((F("  BASIC_PATTERNS    = %d"), BASIC_PATTERNS));
  DBGOUT((F("  CUSTOM_PATTERNS   = %d"), CUSTOM_PATTERNS));
  DBGOUT((F("  EXTERN_PATTERNS   = %d"), EXTERN_PATTERNS));
  DBGOUT((F("  EXTERNAL_COMM     = %d"), EXTERNAL_COMM));
  DBGOUT((F("  NUM_PLUGIN_TRACKS = %d"), NUM_PLUGIN_TRACKS));
  DBGOUT((F("  NUM_PLUGIN_LAYERS = %d"), NUM_PLUGIN_LAYERS));
  DBGOUT((F("  EE_PATTERN_START  = %d"), FLASHOFF_PATTERN_START));
  DBGOUT((F("  EE_PATTERN_END    = %d"), FLASHOFF_PATTERN_END));
  DBGOUT((F("  EEPROM_FREE_BYTES = %d"), EEPROM_FREE_BYTES));

  SetupBrightControls();
  SetupDelayControls();
  SetupEModeControls();
  SetupColorControls();
  SetupCountControls();
  SetupTriggerControls();
  SetupPatternControls();

  CheckForPatterns();     // read internal patterns (if any) and current pattern number
  pCustomCode->setup();   // custom initialization here (external communications setup)

  GetCurPattern(cmdStr);  // get pattern string corresponding that that pattern number
  CheckExecCmd(cmdStr);   // load that pattern into the engine: ready to be displayed

  BlinkStatusLED(0, 2); // indicate success
  DBGOUT((F("** Setup complete **")));
}

//*********************************************************************************************

void loop()
{
  if (!pCustomCode->loop())
  {
    // check physical controls for changes
    CheckBrightControls();
    CheckDelayControls();
    CheckEModeControls();
    CheckColorControls();
    CheckCountControls();
    CheckTriggerControls();
    CheckPatternControls();
  }
  CheckExecCmd(cmdStr); // load any new pattern into the engine

  // if enabled: display new pixel values if anything has changed
  if (doUpdate && pPixelNutEngine->updateEffects()) ShowPixels();
}

#endif
//*********************************************************************************************
