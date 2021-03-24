// PixelNut! FireFly Application
// Copyright © 2017, Greg de Valois, www.devicenut.com
//
// This application is built specifically for the "FireFly" PixelNut! product:
// A Teensy LC processor module from PJRC (www.pjrc.com), which is mounted on a
// circuit board along with a accelerometer and a battery charger, powered by a
// 2000 mA lithium battery, installed inside a mason jar with a power button and
// a mode button, and attached to a strand of WS2812B LEDs.

/*---------------------------------------------------------------------------------------------
 This is free software: you can redistribute it and/or modify it under the terms of the GNU
 Lesser General Public License as published by the Free Software Foundation, version 3 or later.
 http://www.gnu.org/licenses/

 This is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
---------------------------------------------------------------------------------------------*/

#include <Arduino.h>            // Standard Arduino includes
#include <NeoPixelShow.h>       // Adafruit's Neopixel Show()
#include <PixelNutLib.h>        // Devicenut's PixelNutLib Library
#include <UIDeviceButton.h>     // Devicenut's DeviceButton Class

// make sure we're compiling for the correct processor
#if !defined(__arm__) || !defined(__MKL26Z64__)
#error("Compile for the Teensy LC processor")
#endif

//*********************************************************************************************
// Constants and Variables that define the application, used by common application code
//*********************************************************************************************

#define DEBUG_OUTPUT            0           // 1 to enable serial output for debugging
#define DEBUG_SIGNON            "PixelNut! FireFly" // name displayed on debug output

#define PIXEL_COUNT             125         // 125 pixels wound around
#define DPIN_PIXELS             17          // drives output pixel data
#define DPIN_LED                13          // drives R-LED for error status
#define DPIN_FADER_BUTTON       12          // toggles fader/player mode

#define XAPP_FIREFLY            1           // enables extended code
#define MAIN_OVERRIDE           1           // with its own setup/loop
#define PATTERN_OVERRIDE        1           // creates its own patterns

//**************************************************************************************************
#include <PixelNutApp.h>                    // Devicenut's PixelNutApp Library (common application code)
//**************************************************************************************************
