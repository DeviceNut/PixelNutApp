//*********************************************************************************************
// Custom Setup & Loop routines (required for multiple physical segment configurations).
//*********************************************************************************************
/*
Copyright (c) 2015-2020, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/
#if defined(XMAIN_ENABLE) && XMAIN_ENABLE && STRANDS_MULTI

static uint16_t pixelBytes[SEGMENT_COUNT];
static byte *pixelArrays[SEGMENT_COUNT];

#if !PIXELS_APA
NeoPixelShow *neoPixels[SEGMENT_COUNT];
#endif

PixelNutEngine *pixelNutEngines[SEGMENT_COUNT];
PixelNutEngine *pPixelNutEngine; // pointer to current engine

#if DEBUG_OUTPUT
void FindSizeOfHeap(void)
{
  int size = 1024;
  void* p;
  void *list[50];
  int count = 0;
  while(1)
  {
    p = malloc(size);
    if (p == NULL)
    {
      DBGOUT((F("HeapSize = %d Kb"), count));
      for (int i = 0; i < count; ++i) free(list[i]);
      break;
    }
    list[count++] = p;
  }
}
#endif

void ShowPixels(int seg)
{
  #if PIXELS_APA
  byte *ptr = pixelArrays[seg];
  int count = pixelCounts[seg];

  digitalWrite(PIXEL_PINS[seg], LOW); // enable this strand
  SPI.beginTransaction(spiSettings);

  // 4 byte start-frame marker
  for (int i = 0; i < 4; i++) SPI.transfer(0x00);

  for (int i = 0; i < count; ++i)
  {
    SPI.transfer(0xFF);
    for (int j = 0; j < 3; j++) SPI.transfer(*ptr++);
  }

  SPI.endTransaction();
  digitalWrite(PIXEL_PINS[seg], HIGH); // disable this strand

  #else
  neoPixels[seg]->show(pixelArrays[seg], pixelBytes[seg]);
  #endif
}

void CheckExecCmd(char *instr)
{
  if (instr[0])
  {
    DBGOUT((F("Exec[%d]: \"%s\""), curSegment, instr));

    PixelNutEngine::Status status = pPixelNutEngine->execCmdStr(instr);
    if (status != PixelNutEngine::Status_Success)
    {
      DBGOUT((F("CmdErr[%d]: %d"), curSegment, status));
      ErrorHandler(2, status, false); // blink for error and continue
    }
    else pCustomCode->pattern();

    instr[0] = 0; // must clear command string after finished
  }
}

void setup()
{
  SetupLED(); // status LED: indicate in setup now
  SetupDBG(); // setup/wait for debug monitor
  // Note: cannot use debug output until above is used,
  // meaning DBGOUT() cannot be used in constructors.

  #if EEPROM_FORMAT
  FlashFormat(); // format entire EEPROM
  pCustomCode->flash(); // custom flash handling
  ErrorHandler(0, 3, true);
  #endif

  #if DEBUG_OUTPUT
  #if defined(DPIN_HANG_SWITCH)
  pinMode(DPIN_HANG_SWITCH, INPUT_PULLUP);
  //DBGOUT((F("Hang switch = %d", digitalRead(DPIN_HANG_SWITCH));
  if (digitalRead(DPIN_HANG_SWITCH) == LOW)
  {
    DBGOUT((F("Hanging...")));
    while(true);
  }
  #endif
  #endif

  #if PIXELS_APA
  for (int i = 0; i < SEGMENT_COUNT; ++i) // config chip select pins
  {
    pinMode(PIXEL_PINS[i], OUTPUT);
    digitalWrite(PIXEL_PINS[i], HIGH);
  }
  SPI.begin(); // initialize SPI library
  #endif
      
  DBG( FindSizeOfHeap(); )

  DisplayConfiguration(); // Display configuration settings

  SetupBrightControls();  // Setup any physical controls present
  SetupDelayControls();
  SetupEModeControls();
  SetupColorControls();
  SetupCountControls();
  SetupTriggerControls();
  SetupPatternControls();

  CheckForPatterns(); // only do this once

  DBGOUT((F(">>> Begin extended setup...")));

  byte lcount, tcount;

  // alloc arrays, turn off pixels, init patterns
  for (int i = 0; i < SEGMENT_COUNT; ++i)
  {
    pixelBytes[i] = PIXEL_COUNTS[i]*3;

    pixelArrays[i] = (byte*)calloc(pixelBytes[i], sizeof(byte));
    if (pixelArrays[i] == NULL)
    {
      DBGOUT((F("Alloc failed for pixel array, segment=%d"), i));
      ErrorHandler(1, 0, true);
    }

    #if !PIXELS_APA
    neoPixels[i] = new NeoPixelShow(PIXEL_PINS[i]);
    if (neoPixels[i] == NULL)
    {
      DBGOUT((F("Alloc failed for neopixel class, segment=%d"), i));
      ErrorHandler(1, 0, true);
    }
    #if defined(ESP32)
    if (!neoPixels[i]->rmtInit(i, pixelBytes[i]))
    {
      DBGOUT((F("Alloc failed for RMT data, segment=%d"), i));
      ErrorHandler(1, 0, true);
    }
    #endif
    #endif

    ShowPixels(i); // turn off pixels

    pixelNutEngines[i] = new PixelNutEngine(pixelArrays[i], PIXEL_COUNTS[i],
                                PIXEL_OFFSET, PIXEL_DIRS[i],
                                NUM_PLUGIN_LAYERS, NUM_PLUGIN_TRACKS);

    if ((pixelNutEngines[i] == NULL) || (pixelNutEngines[i]->pDrawPixels == NULL))
    {
      DBGOUT((F("Failed to alloc pixel engine buffers, segment=%d"), i));
      ErrorHandler(2, PixelNutEngine::Status_Error_Memory, true);
    }
    pPixelNutEngine = pixelNutEngines[i];

    FlashSetSegment(curSegment = i);
    FlashStartup();         // get curPattern and settings from flash, set engine properties
    GetCurPattern(cmdStr);  // get pattern string corresponding to that pattern number
    CheckExecCmd(cmdStr);   // load that pattern into the engine: ready to be displayed
  }

  FlashSetSegment(curSegment = 0);
  pPixelNutEngine = pixelNutEngines[0];

  pCustomCode->setup();   // custom initialization here (external communications setup)

  #if defined(ESP32)
  randomSeed(esp_random()); // should be called after BLE/WiFi started
  #else
  // set seed to value read from unconnected analog port
  randomSeed(analogRead(APIN_SEED));
  #endif

  DBG( FindSizeOfHeap(); )

  BlinkStatusLED(0, 2);  // indicate success
  DBGOUT((F("** Setup complete **")));
}

void loop()
{
  pCustomCode->loop();

  // check physical controls for changes
  CheckBrightControls();
  CheckDelayControls();
  CheckEModeControls();
  CheckColorControls();
  CheckCountControls();
  CheckTriggerControls();
  CheckPatternControls();

  CheckExecCmd(cmdStr); // load any new pattern into the engine

  // if enabled: display new pixel values if anything has changed
  if (doUpdate)
  {
    for (int i = 0; i < SEGMENT_COUNT; ++i)
      if (pixelNutEngines[i]->updateEffects())
        ShowPixels(i);
  }
}

extern PixelNutEngine *pixelNutEngines[];

// could be called by internal controls
void SwitchSegments(byte segindex)
{
  if (segindex < SEGMENT_COUNT)
  {
    curSegment = segindex;
    DBGOUT((F("Switch to segment=%d"), curSegment));
    FlashSetSegment(curSegment);
    pPixelNutEngine = pixelNutEngines[curSegment];
  }
}

#if (EXTERNAL_COMM && defined(APPCMDS_EXTEND) && APPCMDS_EXTEND)

class AppCommandsX : public AppCommands
{
public:

  virtual bool cmdHandler(char *instr)
  {
    //DBGOUT((F("CmdHandlerX: \"%s\""), instr));

    switch (instr[0])
    {
      case '#': // client is switching physical segments
      {
        byte segindex = *(instr+1)-0x30; // convert ASCII digit to value
        SwitchSegments(segindex);
        break;
      }
      default: return AppCommands::cmdHandler(instr);
    }

    return true;
  }
};

AppCommandsX appCmdX; // extended class instance
AppCommands *pAppCmd = &appCmdX;

#endif // (EXTERNAL_COMM && APPCMDS_EXTEND)
#endif // XMAIN_ENABLE && STRANDS_MULTI
//*********************************************************************************************
