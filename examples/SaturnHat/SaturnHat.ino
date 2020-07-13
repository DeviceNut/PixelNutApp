// PixelNut! Saturn Hat Application
// Copyright © 2017, Greg de Valois, www.devicenut.com
//
// This application is an example of a custom built PixelNut! hat product:
// A Teensy 3.2 processor module from PJRC (www.pjrc.com), mounted on a hat
// with a strand of WS2812B LEDs and one or more pixel rings from Adafruit,
// powered by a battery, and controlled by the PixelNut! phone application.
//
// These can be special ordered from: www.pixelnut.io

/*---------------------------------------------------------------------------------------------
 This is free software: you can redistribute it and/or modify it under the terms of the GNU
 Lesser General Public License as published by the Free Software Foundation, version 3 or later.
 http://www.gnu.org/licenses/

 This is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
---------------------------------------------------------------------------------------------*/

#include <Arduino.h>            // Standard Arduino includes
#include <NeoPixelShow.h>       // Adafruit's Neopixel Show()
#include <BluefruitStrs.h>      // Devicenut's BluefruitStrs Library
#include <PixelNutLib.h>        // Devicenut's PixelNutLib Library

// make sure we're compiling for the correct processor
#if !defined(__arm__) || !defined(__MK20DX256__)
#error("Compile for the Teensy 3.2 processor")
#endif

//*********************************************************************************************
// Constants and Variables that define the application, used by common application code
//*********************************************************************************************

#define DEBUG_OUTPUT            0           // 1 to enable serial output for debugging
#define DEBUG_SIGNON            "PixelNut! Saturn Hat" // name displayed on debug output

// when first flash a new device, set this to 1 to clear the EEPROM, then reset to 0
#define EEPROM_FORMAT           0           // 1 to write 0 to entire flash data space
                                            // MUST be performed once on new hardware

#define PIXEL_COUNT             74                      // strip of 53 + 3 rings of 7
#define SEGMENT_COUNT           4                       // number of logical segments
#define SEGMENT_INFO            "0 53 53 7 60 7 67 7"   // segment offsets/lengths: 3 rings

#define DPIN_PIXELS             1           // drives output pixel data (from battery)

#define STRLEN_PATTERNS         300         // maxlen of pattern strings (including ending 0)
#define NUM_PLUGIN_LAYERS       16          // number of plugins that can be combined at once
#define NUM_PLUGIN_TRACKS       8           // layers that have unique properties (redraw pixels)

#define BLE_BLUEFRUIT           1           // include bluefruit bluetooth handling code

//**************************************************************************************************
#include <PixelNutApp.h>        // Devicenut's PixelNutApp Library (common application code)
//**************************************************************************************************

