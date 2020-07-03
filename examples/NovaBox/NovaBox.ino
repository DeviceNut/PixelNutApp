// PixelNut! Nova Box Application
// Copyright © 2017, Greg de Valois, www.devicenut.com
//
// This application is built specifically for the "Nova Box" PixelNut! product:
// A Teensy 3.2 processor module from PJRC (www.pjrc.com), in a box with an physical
// controls, powered from an adapter, and attached to a strand of 300 WS2812B LEDs.

/*---------------------------------------------------------------------------------------------
 This is free software: you can redistribute it and/or modify it under the terms of the GNU
 Lesser General Public License as published by the Free Software Foundation, version 3 or later.
 http://www.gnu.org/licenses/

 This is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
---------------------------------------------------------------------------------------------*/

#include <Arduino.h>            // Standard Arduino includes
#include <NeoPixelShow.h>       // Adafruit's Neopixel Show()
#include <UIDeviceAnalog.h>     // Devicenut's Analog Class
#include <UIDeviceButton.h>     // Devicenut's Button Class
#include <PixelNutLib.h>        // Devicenut's PixelNutLib Library
#include "MyPatterns.h"         // Customized effect patterns

// make sure we're compiling for the correct processor
#if !defined(__arm__) || !defined(__MK20DX256__)
#error("Compile for the Teensy 3.2 processor")
#endif

//*********************************************************************************************
// Constants and Variables that define the application, used by common application code
//*********************************************************************************************

#define DEBUG_OUTPUT            0           // 1 to enable serial output for debugging
#define DEBUG_SIGNON            "PixelNut! Nova Box" // name displayed on debug output

#define MAX_BRIGHTNESS          100         // allow for the maximum brightness
#define DELAY_OFFSET            0           // no additional delay needed
#define PIXEL_COUNT             300         // 5 meter strand of pixels
#define DPIN_PIXELS             20          // drives output pixel data (@5v)
#define DPIN_LED                13          // on-board red LED for status
#define DPIN_PATTERN_BUTTON     2           // preset selection button
#define DPIN_PAT_BUTTON_PREV    6           // prev pattern select button
#define DPIN_TRIGGER_BUTTON     4           // trigger control button
#define APIN_SEED               A0          // used to seed randomizer
#define APIN_DELAY_POT          A1          // delay control potentiometer
#define APIN_BRIGHT_POT         A3          // brightness control potentiometer
#define BRIGHT_POT_BACKWARDS    0           // brightness pot NOT wired backwards
#define DELAY_POT_BACKWARDS     1           // delay pot IS wired backwards

//**************************************************************************************************
#include <PixelNutApp.h>        // Devicenut's PixelNutApp Library (common application code)
//**************************************************************************************************
