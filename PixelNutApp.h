// PixelNutApp: Include All Application Code
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if defined(ARDUINO)
#include "includes/defines.h"       // Global Defines
#include "includes/custom.h"        // Base Custom Class
#include "includes/globals.h"       // Global Variables
                                    // Helper Functions:
#include "includes/led.h"           // LED Indicator Control
#include "includes/dbgerr.h"        // Debug/Error Handling
#include "includes/flash.h"         // Flash Storage Handling
#include "includes/select.h"        // Preset Patterns Selection
#include "includes/appcmds.h"       // Application Commands
#include "includes/wifi-esp32.h"    // WiFi using ESP32 
#include "includes/wifi-particle.h" // WiFi using Particle 
#include "includes/ble-bluefruit.h" // Bluefriuit Bluetooth
#include "includes/segments.h"      // Logical Segment Support
                                    // Physical Controls:
#include "includes/pattern.h"       // Sets the current preset pattern string
#include "includes/trigger.h"       // Triggers effects in current pattern string 
#include "includes/emode.h"         // Sets extern color/count property mode enable
#include "includes/delay.h"         // Sets global delay offset applied to all effects
#include "includes/bright.h"        // Sets global max brightness applied to all effects
#include "includes/color.h"         // Sets color hue/white properties in pixelnut engine
#include "includes/count.h"         // Sets pixel count property in pixelnut engine
#include "includes/custvar.h"       // Sets global reference to custom code handler
                                    // Main Routines:
#include "includes/main.h"          // setup() and loop() functions

#elif defined(SPARK)
#include "defines.h"
#include "globals.h"
#include "custom.h"
#include "led.h"
#include "dbgerr.h"
#include "flash.h"
#include "select.h"
#include "appcmds.h"
#include "particle-wifi.h"
#include "segments.h"
#include "pattern.h"
#include "trigger.h"
#include "emode.h"
#include "delay.h"
#include "bright.h"
#include "color.h"
#include "count.h"
#include "main.h"
#endif
