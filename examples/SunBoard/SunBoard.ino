// PixelNut! Sun Board Application
// Copyright © 2017, Greg de Valois, www.devicenut.com
//
// This application is built specifically for the "Sun Board" PixelNut! product:
// A Teensy LC processor module from PJRC (www.pjrc.com), which is mounted on a
// circuit board along with a battery charger from Adafruit (www.adafruit.com),
// powered by a 2000-2500 mA lithium battery, uses 4 buttons for control, and is
// attached to a 1 meter strand of 144 WS2812B LEDs.

/*---------------------------------------------------------------------------------------------
 This is free software: you can redistribute it and/or modify it under the terms of the GNU
 Lesser General Public License as published by the Free Software Foundation, version 3 or later.
 http://www.gnu.org/licenses/

 This is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
---------------------------------------------------------------------------------------------*/

#include <Arduino.h>            // Standard Arduino includes
#include <NeoPixelShow.h>       // Adafruit's Neopixel Show()
#include <UIDeviceButton.h>     // Devicenut's Button Class
#include <PixelNutLib.h>        // Devicenut's PixelNutLib Library
#include "MyPatterns.h"         // Customized effect patterns

// make sure we're compiling for the correct processor
#if !defined(__arm__) || !defined(__MKL26Z64__)
#error("Compile for the Teensy LC processor")
#endif

//*********************************************************************************************
// Constants that define this application, used by the common application code
//*********************************************************************************************

#define DEBUG_OUTPUT            0           // 1 to enable serial output for debugging
#define DEBUG_SIGNON            "PixelNut! Sun Board" // name displayed on debug output

#define MAX_BRIGHTNESS          85          // limit brightness for longer battery life
#define DELAY_OFFSET            -15         // speed it up a little
#define PIXEL_COUNT             144         // can support up to 240
#define DPIN_PIXELS             17          // drives output pixel data (from battery)
#define DPIN_LED                13          // on-board red LED for status
#define DPIN_PATTERN_BUTTON     9           // pattern selection button
#define DPIN_TRIGGER_BUTTON     11          // trigger control button
#define DPIN_DELAY_BUTTON       12          // delay selection button
#define DPIN_BRIGHT_BUTTON      26          // brightness selection button
#define APIN_SEED               A0          // used to seed randomizer

//**************************************************************************************************
#include <PixelNutApp.h>        // Devicenut's PixelNutApp Library (common application code)
//**************************************************************************************************
