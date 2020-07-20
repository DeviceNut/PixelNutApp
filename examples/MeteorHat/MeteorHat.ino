// PixelNut! Meteor Hat Application
// Copyright © 2017, Greg de Valois, www.devicenut.com
//
// This application is built specifically for the "Meteor Hat" PixelNut! products:
// A Feather 32u4 processor module from Adafruit (www.adafruit.com), mounted on a hat
// with a strand of WS2812B LEDs, powered by a battery, and uses 3 buttons for control.

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

// make sure we're compiling for the correct processor
#if !defined(__AVR__) || !defined(__AVR_ATmega32U4__)
#error("Compile for the Feather 32u4 module")
#endif

//*********************************************************************************************
// Constants that define this application, used by the common application code
//*********************************************************************************************

#define MAX_BRIGHTNESS          85          // limit brightness for longer battery life
#define DELAY_OFFSET            20          // additional msecs delay: few pixels
#define PIXEL_COUNT             32          // can support up to 64 pixels
#define DPIN_PIXELS             12          // drives output pixel data (from battery)
#define DPIN_PATTERN_BUTTON     3           // pattern select button
#define DPIN_DELAY_BUTTON       11          // delay select button
#define DPIN_BRIGHT_BUTTON      2           // brightness select button

#define PATTERN_JULY4           1           // enable extended "July 4th" pattern
#define CUSTOM_PATTERNS         1           // enables extended patterns in general
#define PATTERN_JULY4           1           // enables "July 4th" pattern specifically

//**************************************************************************************************
#include <PixelNutApp.h>        // Devicenut's PixelNutApp Library (common application code)
//**************************************************************************************************
