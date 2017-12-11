// PixelNutApp Main Setup and Loop Routines
//
// Uses global variables: 'pPixelNutEngine', 'pCustomCode', 'pNeoPixels', 'pPixelBytes',
//                        'pixelBytes', 'engineStatus', 'doUpdate'.
//
// Calls common global control/command functions.
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if (EXTERNAL_COMM && (EEPROM_BYTES == 0))
#error("Must have EEPROM to use external communications")
#endif

#if !MAIN_OVERRIDE

void setup()
{
  SetupLEDs(); // status LED: indicate in setup now
  SetupDebugInterface(); // setup/wait for debug monitor
  // Note: cannot use debug output until above is used,
  // meaning DBGOUT() cannot be used in constructors.

  // set seed to value read from unconnected analog port
  randomSeed(analogRead(APIN_SEED));

  // turn off all pixels
  memset(pPixelData, 0, pixelBytes);
  pNeoPixels->show(pPixelData, pixelBytes);

  #if EEPROM_FORMAT
  FlashFormat();
  #endif

  if (pPixelNutEngine->pDrawPixels == NULL)
  {
    DBGOUT((F("Failed to alloc pixel engine buffers")));
    ErrorHandler(2, PixelNutEngine::Status_Error_Memory, true);
  }

  #if EXTERNAL_COMM
  FlashStartup(); // get saved settings from flash
  #endif
  #if BLUETOOTH_COMM
  SPI.begin();    // initialize SPI library
  #endif

  DBGOUT((F("Configuration Settings:")));
  #if !EXTERNAL_COMM
  DBGOUT((F("  MAX_BRIGHTNESS    = %d"), MAX_BRIGHTNESS));
  DBGOUT((F("  DELAY_OFFSET      = %d"), DELAY_OFFSET));
  #endif
  DBGOUT((F("  PIXEL_COUNT       = %d"), PIXEL_COUNT));
  DBGOUT((F("  STRAND_COUNT      = %d"), STRAND_COUNT));
  DBGOUT((F("  SEGMENT_COUNT     = %d"), SEGMENT_COUNT));
  DBGOUT((F("  CUSTOM_PATTERNS   = %d"), CUSTOM_PATTERNS));
  DBGOUT((F("  BASIC_PATTERNS    = %d"), BASIC_PATTERNS));
  DBGOUT((F("  STRLEN_PATTERNS   = %d"), STRLEN_PATTERNS));
  DBGOUT((F("  NUM_PLUGIN_LAYERS = %d"), NUM_PLUGIN_LAYERS));
  DBGOUT((F("  NUM_PLUGIN_TRACKS = %d"), NUM_PLUGIN_TRACKS));
  DBGOUT((F("  EEPROM_FREE_BYTES = %d"), EEPROM_FREE_BYTES));

  pCustomCode->setup(); // custom initialization here
  // this is where the external communications are setup

  SetupBrightControls();
  SetupDelayControls();
  SetupEModeControls();
  SetupColorControls();
  SetupCountControls();
  SetupTriggerControls();
  SetupPatternControls();

  CheckForPatterns();   // read internal patterns (if any) and current pattern number
  GetCurPattern();      // get pattern string cooresponding that that pattern number
  CheckExecCmd();       // load that pattern into the engine: ready to be displayed

  BlinkStatusLED(0,1);  // indicate success
  DBGOUT((F("** Setup complete **")));
}

//*********************************************************************************************

void CheckExecCmd()
{
  if (cmdStr[0]) // if have new command for engine
  {
    DBGOUT((F("Exec: \"%s\""), cmdStr));

    engineStatus = pPixelNutEngine->execCmdStr(cmdStr);
    cmdStr[0] = 0; // must clear command string after finished

    if (engineStatus != PixelNutEngine::Status_Success)
    {
      DBGOUT((F("CmdErr: %d"), engineStatus));
      ErrorHandler(2, engineStatus, false); // blink for error and continue
    }
    else pCustomCode->display();
  }
}

void loop()
{
  if (!pCustomCode->control())
  {
    // check physical controls for changes
    CheckBrightControls();
    CheckDelayControls();
    CheckEModeControls();
    CheckColorControls();
    CheckCountControls();
    CheckTriggerControls();
    CheckPatternControls();

    CheckExecCmd(); // load any new pattern into the engine
  }

  // if enabled: display new pixel values if anything has changed
  if (doUpdate && pPixelNutEngine->updateEffects())
    pNeoPixels->show(pPixelData, pixelBytes);
}

#endif
//*********************************************************************************************
