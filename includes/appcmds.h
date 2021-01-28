// PixelNutApp Application Command Handling
//
// Uses global variables: 'pPixelNutEngine', 'curPattern'.
// Calls global routines: 'FlashSetPattern', 'FlashSetStr', 'FlashSetBright',
//                        'FlashSetDelay', 'FlashSetExterns', 'FlashSetXmode',
//                        'FlashSetForce', 'FlashSetProperties', 'ErrorHandler'.
//========================================================================================
/*
Copyright (c) 2015-2020, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if EXTERNAL_COMM

class AppCommands
{
public:

  virtual bool cmdHandler(char *instr)
  {
    //DBGOUT((F("cmdHandler: \"%s\""), instr));

    if (*instr) switch (instr[0])
    {
      default:
      {
        if (saveStrIndex >= 0) // save part of pattern into flash
        {
          FlashSetStr(instr, saveStrIndex);
          saveStrIndex += strlen(instr);
        }
        else if (isdigit(instr[0])) // request to change pattern
        {
          curPattern = atoi(instr);
          FlashSetPattern(curPattern);
          GetCurPattern(instr);
          useCmdStr = true;
        }
        else if (isalpha(instr[0])) // have engine command
          useCmdStr = true;

        else return false;
        break;
      }
      case '.': // start/stop saving a pattern into flash
      {
        if ( saveStrIndex < 0)  // begin sequence
             saveStrIndex = 0;
        else saveStrIndex = -1; // end sequence
        break;
      }
      case '?': // returns current settings
      {
        #if (SEGMENT_COUNT > 1)
        if (instr[1] == 'S') // send info about segments
        {
          DBGOUT((F("Segment Info:  %s"), SEGMENT_INFO));
          if (!pCustomCode->sendReply((char*)SEGMENT_INFO)) return false;

          char outstr[FLASH_SEG_LENGTH * 5]; // max length of each value is 4 + space
          for (int i = 0; i < SEGMENT_COUNT; ++i)
          {
            FlashSetSegment(i);

            outstr[0] = 0;
            for (int j = 0; j < FLASH_SEG_LENGTH; ++j)
              AddNumToStr(outstr, FlashGetValue(j));

            DBGOUT((F("FlashVals[%d]: %s"), i, outstr));

            if (!pCustomCode->sendReply(outstr)) return false;
          }

          FlashSetSegment(curSegment = 0); // MUST start with segment 0
          break;
        }
        #endif

        #if CUSTOM_PATTERNS
        if (instr[1] == 'P') // about internal patterns
        {
          DBGOUT((F("Patterns:  %d"), codePatterns));

          char outstr[STRLEN_PATTERNS];
          for (int i = 0; i < codePatterns; ++i)
          {
            strcpy_P(outstr, customPatterns[i]);
            if (!pCustomCode->sendReply((char*)customPnames[i]) ||
                !pCustomCode->sendReply((char*)customPhelp[i])  ||
                !pCustomCode->sendReply(outstr))
                return false;
          }
          break;
        }
        #endif

        char outstr[100];

        DBGOUT((F("Info Line #1")));
        DBGOUT((F("  Title=P!")));
        strcpy(outstr, "P! ");

        #if STRANDS_MULTI         // multiple physical strands
        DBGOUT((F("  Lines=2")));
        AddNumToStr(outstr, 2);   // number of lines to be sent
        #elif (SEGMENT_COUNT > 1) // multiple logical segments
        DBGOUT((F("  Lines=3")));
        AddNumToStr(outstr, 3);   // includes line 3
        #else                     // single strand and segment
        DBGOUT((F("  Lines=4")));
        AddNumToStr(outstr, 4);   // includes lines 3 & 4
        #endif

        if (!pCustomCode->sendReply(outstr)) return false;

        DBGOUT((F("Info Line #2")));
        outstr[0] = 0;

        DBGOUT((F("  Segments:    %d"), SEGMENT_COUNT));        // number of segments
        AddNumToStr(outstr, SEGMENT_COUNT);

        DBGOUT((F("  CurPattern:  %d"), curPattern));
        AddNumToStr(outstr, curPattern);
        DBGOUT((F("  NumPatterns: %d"), codePatterns));         // number of custom patterns
        AddNumToStr(outstr, codePatterns);

        #if !EXTERN_PATTERNS
        DBGOUT((F("  Features:    %d"), 0x01)); // cannot use external patterns
        AddNumToStr(outstr, (0x01 | (PIXEL_COUNT << 4)));
        #else
        DBGOUT((F("  Features:    %d"), 0)); // no special features
        AddNumToStr(outstr, (PIXEL_COUNT << 4));
        #endif

        DBGOUT((F("  MultiStrand: %d"), STRANDS_MULTI));
        AddNumToStr(outstr, STRANDS_MULTI);

        DBGOUT((F("  CmdStrLen:   %d"), STRLEN_PATTERNS));      // maxlen of commands/patterns
        AddNumToStr(outstr, STRLEN_PATTERNS);

        if (!pCustomCode->sendReply(outstr)) return false;

        #if !STRANDS_MULTI
        DBGOUT((F("Info Line #3")));
        DBGOUT((F("  PixelCount:  %d"), PIXEL_COUNT));          // total number of pixels
        DBGOUT((F("  LayerCount:  %d"), NUM_PLUGIN_LAYERS));    // max number of layers
        DBGOUT((F("  TrackCount:  %d"), NUM_PLUGIN_TRACKS));    // max number of tracks
        DBGOUT((F("  MaxBright:   %d"), pPixelNutEngine->getMaxBrightness()));
        DBGOUT((F("  DelayOffset: %d"), pPixelNutEngine->getDelayOffset()));

        outstr[0] = 0;
        AddNumToStr(outstr, PIXEL_COUNT);
        AddNumToStr(outstr, NUM_PLUGIN_LAYERS);
        AddNumToStr(outstr, NUM_PLUGIN_TRACKS);
        AddNumToStr(outstr, pPixelNutEngine->getMaxBrightness());
        AddNumToStr(outstr, pPixelNutEngine->getDelayOffset());

        if (!pCustomCode->sendReply(outstr)) return false;
        #endif

        #if (SEGMENT_COUNT <= 1)
        DBGOUT((F("Info Line #4")));
        DBGOUT((F("  ExtMode: %d"), pPixelNutEngine->getPropertyMode()));
        DBGOUT((F("  ExtHue:  %d"), pPixelNutEngine->getPropertyHue()));
        DBGOUT((F("  ExtWht:  %d"), pPixelNutEngine->getPropertyWhite()));
        DBGOUT((F("  ExtCnt:  %d"), pPixelNutEngine->getPropertyCount()));
        DBGOUT((F("  Force:   %d"), FlashGetForce()));

        outstr[0] = 0;
        AddNumToStr(outstr, pPixelNutEngine->getPropertyMode());
        AddNumToStr(outstr, pPixelNutEngine->getPropertyHue());
        AddNumToStr(outstr, pPixelNutEngine->getPropertyWhite());
        AddNumToStr(outstr, pPixelNutEngine->getPropertyCount());
        AddNumToStr(outstr, FlashGetForce()); // not backward compatible

        if (!pCustomCode->sendReply(outstr)) return false;
        #endif

        break;
      }
      case '%': // set maximum brightness level
      {
        pPixelNutEngine->setMaxBrightness(atoi(instr+1));
        FlashSetBright();
        break;
      }
      case ':': // set delay offset
      {
        pPixelNutEngine->setDelayOffset( atoi(instr+1) );
        FlashSetDelay();
        break;
      }
      case '[': // pause display updating
      {
        doUpdate = false;
        break;
      }
      case ']': // resume display updating
      {
        doUpdate = true;
        break;
      }
      case '_': // set external mode on/off
      {
        FlashSetXmode(atoi(instr+1) != 0);
        FlashSetProperties();
        break;
      }
      case '=': // set color hue/white and count properties
      {
        ++instr; // skip past '='
        short hue = atoi(instr);

        instr = skipNumber(instr); // skip number digits
        instr = skipSpaces(instr); // skip leading spaces
        byte wht = atoi(instr);

        instr = skipNumber(instr); // skip number digits
        instr = skipSpaces(instr); // skip leading spaces
        byte cnt = atoi(instr);

        FlashSetExterns(hue, wht, cnt);
        FlashSetProperties();
        break;
      }
      case '!': // causes a trigger given specified force
      {
        short force = atoi(instr+1);
        pPixelNutEngine->triggerForce(force);
        FlashSetForce(force);
        break;
      }
      case '@': // change the device name
      {
        ++instr; // skip '@'
        instr[MAXLEN_DEVICE_NAME] = 0; // limit length

        DBGOUT((F("Seting device name: \"%s\""), instr));
        if (!pCustomCode->setName(instr)) return false;
        break;
      }
    }

    return true;
  }

  bool execCmd(char *cmdstr)
  {
    char *instr = cmdstr;

    if (saveStrIndex < 0) instr = skipSpaces(instr); // skip leading spaces
    // else need those separating spaces while in sequence

    useCmdStr = false;
    bool success = cmdHandler(instr);

    //DBGOUT((F("ExecCmd: success=%d douse=%d"), success, useCmdStr));

    if (!useCmdStr) cmdstr[0] = 0; // avoid executing this
    if (!success) ErrorHandler(4, 1, false);

    return useCmdStr;
  }

  char* skipSpaces(char *instr)
  {
    while (*instr == ' ') ++instr;   // skip spaces
    return instr;
  }

  char* skipNumber(char *instr)
  {
    while (isdigit(*instr)) ++instr; // skip digits
    return instr;
  }

protected:

  int16_t saveStrIndex = -1;
  bool useCmdStr = false;

  void AddNumToStr(char *outstr, int value)
  {
    char numstr[10];
    itoa(value, numstr, 10);
    strcat(outstr, numstr);
    strcat(outstr, " ");
  }
};

#if defined(APPCMDS_EXTEND) && APPCMDS_EXTEND
extern AppCommands *pAppCmd;
#else
AppCommands appCmd; // create instance of AppCommands to handle user commands
AppCommands *pAppCmd = &appCmd; // pointer used by communications classes
#endif

#endif // EXTERNAL_COMM

//========================================================================================
