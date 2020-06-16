// PixelNutApp Global Definitions
// Insure certain required definitions are set to defaults.
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#define DEFAULT_DEVICE_NAME     "PixelNutDevice"
#define MAXLEN_DEVICE_NAME      16          // maxlen for device name
#define PREFIX_DEVICE_NAME      "P!"        // for name to be recognized
#define PREFIX_LEN_DEVNAME      2           // length of this prefix

#if !PIXELS_APA
#define PIXELS_APA              0           // define default value
#endif

#if PIXELS_APA
#if !defined(SPI_SETTINGS_FREQ)
#define SPI_SETTINGS_FREQ       4000000     // use fastest speed by default
#endif
#endif

#if !defined(PIXEL_OFFSET)
#define PIXEL_OFFSET            0           // start drawing at the first pixel
#endif

#if !defined(DIRECTION_UP)
#define DIRECTION_UP            true        // draw from start to end by default
#endif

#if !defined(MAX_BRIGHTNESS)
#define MAX_BRIGHTNESS          100         // default is to allow for maximum brightness
#endif

#if BLUEFRUIT_BLE
#if defined(__AVR_ATmega32U4__)
#define BLUEFRUIT_SPI_CS        8           // assign pins for Bluefruit Micro
#define BLUEFRUIT_SPI_IRQ       7           // ** hardcoded for this module **
#define BLUEFRUIT_SPI_RST       4
#else                                       // must override if change pins:
#define BLUEFRUIT_SPI_CS        10          // wire pins for Bluefruit SPI Friend
#define BLUEFRUIT_SPI_IRQ       9           //  (also connect MOSI, MISO, SCK pins
#define BLUEFRUIT_SPI_RST       8           //   to pins 11, 12, 13 respectively)
#endif
#define BLE_COMM                1           // using bluetooth BLE
#endif

#if PARTICLE_WIFI
#define WIFI_COMM               1           // using WiFi
#endif

#if (BLE_COMM || WIFI_COMM)
#define EXTERNAL_COMM           1           // external communications
#endif

#if !EXTERNAL_COMM
#define EXTERNAL_COMM           0           // define default value
#endif

#if !defined(STRANDS_MULTI)
#define STRANDS_MULTI           0           // have a single strand
#endif

#if !defined(SEGMENT_COUNT)
#define SEGMENT_COUNT           1           // have a single segment
#endif

#if (!STRANDS_MULTI && (SEGMENT_COUNT > 1)) // to support logical segments:
#define APPCMDS_OVERRIDE        1           // must extend AppCommands class
#define PIXENGINE_OVERRIDE      1           // must extend PixelNutEngine class
#endif

#if !defined(CUSTOM_CODE)
#define CUSTOM_CODE             0           // define default value
#endif

#if !defined(APPCMDS_OVERRIDE)
#define APPCMDS_OVERRIDE        0           // define default value
#endif

#if !defined(PIXENGINE_OVERRIDE)
#define PIXENGINE_OVERRIDE      0           // define default value
#endif

#if !defined(CUSTOM_PATTERNS)
#define CUSTOM_PATTERNS         !EXTERNAL_COMM  // must use internal patterns if no external client
#endif

#if !defined(EXTERN_PATTERNS)
#define EXTERN_PATTERNS         EXTERNAL_COMM   // use external (client) patterns if possible
#endif

//========================================================================================
