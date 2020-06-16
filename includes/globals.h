// PixelNutApp Global Variables
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

// required variables:
char cmdStr[STRLEN_PATTERNS];               // command & pattern string
byte codePatterns = 0;                      // number of internal patterns
byte curPattern = 1;                        // current pattern (1..codePatterns)
bool doUpdate = true;                       // false to not update display
byte curSegment = 0;                        // index of current segment

#if CUSTOM_PATTERNS
#if EXTERNAL_COMM
extern const char* const customPnames[];
extern const char* const customPhelp[];
#endif
extern const char* const customPatterns[];
#endif

#if !SHOWPIX_OVERRIDE
byte pixelArray[PIXEL_COUNT*3];             // static allocation of pixel strip
byte *pPixelData = pixelArray;
#endif

#if !SHOWPIX_OVERRIDE && !PIXELS_APA
NeoPixelShow neoPixels = NeoPixelShow(DPIN_PIXELS);
NeoPixelShow *pNeoPixels = &neoPixels;
#endif

#if PIXELS_APA
#include <SPI.h>
SPISettings spiSettings(SPI_SETTINGS_FREQ, MSBFIRST, SPI_MODE0);
PixelValOrder pixorder = {2,1,0}; // mapping of (RGB) to (BRG) for APA102
#else
PixelValOrder pixorder = {1,0,2}; // mapping of (RGB) to (GRB) for WS2812B
#endif
PixelNutSupport pixelNutSupport = PixelNutSupport((GetMsecsTime)millis, &pixorder);

#if !PIXENGINE_OVERRIDE
PixelNutEngine pixelNutEngine = PixelNutEngine(pPixelData, PIXEL_COUNT, PIXEL_OFFSET, DIRECTION_UP, NUM_PLUGIN_LAYERS, NUM_PLUGIN_TRACKS);
PixelNutEngine *pPixelNutEngine = &pixelNutEngine;
#else
extern PixelNutEngine *pPixelNutEngine;
#endif

#if !PLUGINS_OVERRIDE
PluginFactory pluginFactory = PluginFactory();
PluginFactory *pPluginFactory = &pluginFactory;
#else
extern PluginFactory *pPluginFactory;
#endif

//========================================================================================
