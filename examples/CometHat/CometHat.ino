// PixelNut! Comet Application
// Copyright © 2017, Greg de Valois, www.devicenut.com
//
// This application is built specifically for the "Comet Hat" PixelNut! product:
// A Bluefruit Micro module from Adafruit (www.adafruit.com), mounted on a hat
// with a strand of WS2812B LEDs, powered by a battery, and controlled by the
// PixelNut! phone application.
//
// To purchase one of these products: www.pixelnut.io

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
#if !defined(__AVR__) || !defined(__AVR_ATmega32U4__)
#error("Compile for the Bluefruit Micro module")
#endif

//*********************************************************************************************
// Constants that define this application, used by the common application code
//*********************************************************************************************

// when first flash a new device, set this to 1 to clear the EEPROM, then reset to 0
#define EEPROM_FORMAT           0           // MUST be performed once on new hardware

#define PIXEL_COUNT             32          // can support up to 64 pixels with the 32u4
#define DPIN_PIXELS             12          // drives output pixel data (@3.3v)
#define APIN_SEED               A0          // used to seed randomizer

#define STRLEN_PATTERNS         100         // maxlen of pattern strings (including ending 0)
#define NUM_PLUGIN_LAYERS       4           // number of plugins that can be combined at once
#define NUM_PLUGIN_TRACKS       3           // layers that have unique properties (redraw pixels)

#define BLUEFRUIT_BLE           1           // include bluefruit bluetooth handling code

//**************************************************************************************************
#include <PixelNutApp.h>        // Devicenut's PixelNutApp Library (common application code)

CustomCode *pCustomCode = &bluetooth; // override custom class with bluetooth handler

//**************************************************************************************************

