// PixelNut! Comet Application
// Copyright © 2017, Greg de Valois, www.devicenut.com
//
// This application is built specifically for the "Comet Hat" PixelNut! product:
// A Bluefruit Micro module from Adafruit (www.adafruit.com), mounted on a hat
// with a strand of WS2812B LEDs, powered by a battery, and controlled by the
// PixelNut! phone application.

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
#define DPIN_PIXELS             12          // drives output pixel data (from battery)

#define BLE_BLUEFRUIT           1           // include bluefruit bluetooth handling code

#define CUSTOM_PATTERNS         1           // enables extended patterns in general
#define PATTERN_JULY4           1           // enables "July 4th" pattern specifically

//**************************************************************************************************
#include <PixelNutApp.h>        // Devicenut's PixelNutApp Library (common application code)
//**************************************************************************************************

