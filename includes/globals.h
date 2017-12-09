// PixelNutApp Global Code Variables
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

// required variables:
char cmdStr[STRLEN_PATTERNS];               // command & pattern string
byte codePatterns = 0;                      // number of internal patterns
byte curPattern = 0;                        // current pattern (1..savedPatterns)
bool doUpdate = true;                       // false to not update display

#if CUSTOM_PATTERNS
extern const char* const customPatterns[];
#if EXTERNAL_COMM
extern const char* const customPnames[];
extern const char* const customPhelp[];
#endif
#endif

#if !DRAWPIXS_OVERRIDE
short pixelBytes = (PIXEL_COUNT*3);         // number of bytes to store pixels
byte pixelArray[PIXEL_COUNT*3];             // static allocation of pixel strip
byte *pPixelData = pixelArray;

NeoPixelShow neoPixels = NeoPixelShow(DPIN_PIXELS);
NeoPixelShow *pNeoPixels = &neoPixels;

PixelValOrder pixorder = {1,0,2}; // mapping of (RGB) to (GRB) for WS8218B
PixelNutSupport pixelNutSupport = PixelNutSupport((GetMsecsTime)millis, &pixorder);
PixelNutEngine::Status engineStatus; // status from command execution
#endif

#if !PIXENGINE_OVERRIDE
PixelNutEngine pixelNutEngine = PixelNutEngine(pPixelData, PIXEL_COUNT, PIXEL_OFFSET, DIRECTION_UP, NUM_PLUGIN_LAYERS, NUM_PLUGIN_TRACKS);
PixelNutEngine *pPixelNutEngine = &pixelNutEngine;
#else
extern PixelNutEngine *pPixelNutEngine;
#endif

#if !PLUGINS_OVERRIDE
#if BASIC_PATTERNS
PluginFactoryCore pluginFactory = PluginFactoryCore();
#else
PluginFactoryAdv pluginFactory = PluginFactoryAdv();
#endif
PluginFactoryCore *pPluginFactory = &pluginFactory;
#else
extern PluginFactoryCore *pPluginFactory;
#endif

//========================================================================================
