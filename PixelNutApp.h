// PixelNutApp: Include All Application Code
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#include "includes/defines.h"       // Global Defines
#include "includes/globals.h"       // Global Variables
#include "includes/custom.h"        // Default Custom Code
                                    // Helper Functions:
#include "includes/leds.h"          // LED Indicator Control
#include "includes/dbgerr.h"        // Debug/Error Handling
#include "includes/flash.h"         // Flash Storage Handling
#include "includes/select.h"        // Preset Patterns Selection
#include "includes/appcmds.h"       // Application Commands
#include "includes/bluetooth.h"     // Bluetooth Communications
#include "includes/segments.h"      // Logical Segment Support
                                    // Physical Controls:
#include "includes/pattern.h"       // Sets the current preset pattern string
#include "includes/trigger.h"       // Triggers effects in current pattern string 
#include "includes/emode.h"         // Sets extern color/count property mode enable
#include "includes/delay.h"         // Sets global delay offset applied to all effects
#include "includes/bright.h"        // Sets global max brightness applied to all effects
#include "includes/color.h"         // Sets color hue/white properties in pixelnut engine
#include "includes/count.h"         // Sets pixel count property in pixelnut engine
                                    // Main Routines:
#include "includes/main.h"          // setup() and loop() functions