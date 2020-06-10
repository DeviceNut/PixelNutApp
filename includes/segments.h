// Support for multiple logical segments (on a single strand).
//
// Defines global variables: 'pPixelNutEngine', 'pAppCmd'.
// Calls global routines: 'FlashSetSegment', 'FlashSetProperties'.
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if (APPCMDS_OVERRIDE && ((STRAND_COUNT <= 1) && (SEGMENT_COUNT > 1)))

class PixelNutEngineX : public PixelNutEngine
{
public:

  PixelNutEngineX(byte *ptr_pixels, uint16_t num_pixels, uint16_t first_pixel, bool goupwards, short num_layers, short num_tracks) :
    PixelNutEngine(ptr_pixels, num_pixels, first_pixel, goupwards, num_layers, num_tracks)
  {
    // NOTE: cannot call DBGOUT from constructor
  }

  void SetTrackSegEnable(byte segindex)
  {
    for (int i = 0; i <= indexTrackStack; ++i)
    {
      PluginTrack *pTrack = (pluginTracks + i);
      pTrack->disable = (pTrack->segIndex != segindex);

      #if DEBUG_OUTPUT
      DBGOUT((F("Track %d: segment=%d enable=%d"), i, segindex, pTrack->disable));
      #endif
    }
  }
};

PixelNutEngineX pixelNutEngineX   = PixelNutEngineX(pPixelData, PIXEL_COUNT, PIXEL_OFFSET, DIRECTION_UP, NUM_PLUGIN_LAYERS, NUM_PLUGIN_TRACKS);
PixelNutEngineX *pPixelNutEngineX = &pixelNutEngineX;
PixelNutEngine *pPixelNutEngine   = pPixelNutEngineX;

void SwitchSegments(byte segindex)
{
  if (segindex < SEGMENT_COUNT)
  {
    curSegment = segindex;
    DBGOUT((F("Switch to segment=%d"), curSegment));
    FlashSetSegment(curSegment);
    pPixelNutEngineX->SetTrackSegEnable(curSegment);
  }
}

#if EXTERNAL_COMM

class AppCommandsX : public AppCommands
{
public:

  virtual bool cmdHandler(char *instr)
  {
    //DBGOUT((F("CmdHandlerX: \"%s\""), instr));

    switch (instr[0])
    {
      case '#': // allows client to switch logical segments
      {
        byte segindex = *(instr+1)-0x30; // convert ASCII digit to value
        SwitchSegments(segindex);
        break;
      }
      case '%': // brightness/delay: affects all segments so set in first segment
      case ':':
      {
        FlashSetSegment(0);
        AppCommands::cmdHandler(instr);
        FlashSetSegment(curSegment);
        break;
      }
      default: return AppCommands::cmdHandler(instr);
    }

    return true;
  }
};

AppCommandsX appCmdX; // extended class instance
AppCommands *pAppCmd = &appCmdX;

#endif // EXTERNAL_COMM

#if BLE_COMM
class CustomCodeX : public Bluetooth
#elif WIFI_COMM
class CustomCodeX : public WiFiNet
#else
class CustomCodeX : public CustomCode
#endif
{
public:

  void pattern(void) // called if have new pattern in engine
  {
    // must update properties for all segments
    for (int i = 0; i < SEGMENT_COUNT; ++i)
    {
      if (i != curSegment) updateProperties(i);
    }
    updateProperties(curSegment);
  }

private:

  void updateProperties(int seg)
  {
      pPixelNutEngineX->SetTrackSegEnable(seg);
      FlashSetSegment(seg);
      FlashSetProperties();
  }
};

CustomCodeX customX;
CustomCode *pCustomCode = &customX;

#endif // (APPCMDS_OVERRIDE && ((STRAND_COUNT <= 1) && (SEGMENT_COUNT > 1)))
//========================================================================================
