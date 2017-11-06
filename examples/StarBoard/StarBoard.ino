// PixelNut! Star Board Application
// Copyright © 2017, Greg de Valois, www.devicenut.com
//
// This application is built specifically for the "Star Board" PixelNut! product:
// A Teensy LC processor module from PJRC (www.pjrc.com), which is mounted on a
// circuit board along with a battery charger and Bluefruit LE SPI Friend module
// from Adafruit (www.adafruit.com), powered by a 2000-2500 mA lithium battery,
// and attached to a 1 meter strand of 144 WS2812B LEDs.
//
// To purchase one of these products: www.pixelnutstore.com//

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
#if !defined(__arm__) || !defined(__MKL26Z64__)
#error("Compile for the Teensy LC processor")
#endif

//*********************************************************************************************
// Constants and Variables that define the application, used by common application code
//*********************************************************************************************

#define DEBUG_OUTPUT            0           // 1 to enable serial output for debugging
#define DEBUG_SIGNON            "PixelNut! Star Board" // name displayed on debug output

// when first flash a new device, set this to 1 to clear the EEPROM, then reset to 0
#define EEPROM_FORMAT           0           // 1 to write 0 to entire flash data space
                                            // MUST be performed once on new hardware

#define PIXEL_COUNT             144         // standard 1 meter strand of pixels
#define DPIN_PIXELS             17          // drives output pixel data (@Vin)
#define DPIN_LED                5           // off-board red LED for status
#define APIN_SEED               A0          // used to seed randomizer

#define BLUETOOTH_COMM          1           // include bluetooth code
#define BLUEFRUIT_SPI_CS        10          // wire pins for Bluefruit SPI Friend
#define BLUEFRUIT_SPI_IRQ       9           //  (also connect MOSI, MISO, SCK pins
#define BLUEFRUIT_SPI_RST       8           //   to pins 11, 12, 13 respectively)

#define STRLEN_PATTERNS         110         // maxlen of pattern strings (including ending 0)
#define NUM_PLUGIN_LAYERS       8           // number of plugins that to be combined at once
#define NUM_PLUGIN_TRACKS       4           // layers that have unique properties (redraw pixels)

//**************************************************************************************************
#include <PixelNutApp.h>        // Devicenut's PixelNutApp Library (common application code)

CustomCode *pCustomCode = &bluetooth; // override custom class with bluetooth handler

//**************************************************************************************************
