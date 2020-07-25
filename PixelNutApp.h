// PixelNutApp: Include All Application Code
/*
Copyright (c) 2015-2020, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if defined(ARDUINO)
#include "includes/defines.h"       // Global Defines
#include "includes/custom.h"        // Base Custom Class
#include "includes/globals.h"       // Global Variables
#include "includes/patterns.h"      // Patterns Strings
                                    // Helper Functions:
#include "includes/led.h"           // LED Indicator Control
#include "includes/dbgerr.h"        // Debug/Error Handling
#include "includes/flash.h"         // Flash Storage Handling
#include "includes/pselect.h"       // Pattern Selection
#include "includes/appcmds.h"       // Application Commands
#include "includes/wifi-esp32.h"    // WiFi using ESP32 
#include "includes/ble-bluefruit.h" // Bluefriuit Bluetooth
#include "includes/segments.h"      // Logical Segment Support
                                    // Physical Controls:
#include "includes/ctrl-pattern.h"  // Sets the current preset pattern string
#include "includes/ctrl-trigger.h"  // Triggers effects in current pattern string 
#include "includes/ctrl-emode.h"    // Sets extern color/count property mode enable
#include "includes/ctrl-delay.h"    // Sets global delay offset applied to all effects
#include "includes/ctrl-bright.h"   // Sets global max brightness applied to all effects
#include "includes/ctrl-color.h"    // Sets color hue/white properties in pixelnut engine
#include "includes/ctrl-count.h"    // Sets pixel count property in pixelnut engine
                                    // Specialized Code:
#include "includes/custvar.h"       // Sets global reference to custom code handler
#include "includes/freqfft.h"       // Performs frequency analysis with FFT
                                    // Main Routines:
#include "includes/main.h"          // setup() and loop() functions
#include "includes/xmain.h"         // Must be used for multiple physical segments
                                    // Extended Effect Plugins:
#include "includes/PNP_Spectra.h"   // Creates spectrum effect with frequency FFT
#include "includes/PNP_Plasma.h"    // Creates plasma effect with Lissajious curves
#include "includes/xplugins.h"      // Supports above extended effects
                                    // Extended Applications (with setup/loop):
#include "includes/xapp_firefly.h"  // FireFly - uses accelerometer

#elif defined(SPARK)
#include "defines.h"
#include "globals.h"
#include "custom.h"
#include "patterns.h"
#include "led.h"
#include "dbgerr.h"
#include "flash.h"
#include "pselect.h"
#include "appcmds.h"
#include "wifi-particle.h"
#include "segments.h"
#include "ctrl-pattern.h"
#include "ctrl-trigger.h"
#include "ctrl-emode.h"
#include "ctrl-delay.h"
#include "ctrl-bright.h"
#include "ctrl-color.h"
#include "ctrl-count.h"
#include "custvar.h"
#include "main.h"
#include "xmain.h"
#include "PNP_Spectra.h"
#include "PNP_Plasma.h"
#include "xplugins.h"
#endif
