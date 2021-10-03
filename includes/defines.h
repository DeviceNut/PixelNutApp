// PixelNutApp Global Definitions
// Insure certain required definitions are set to defaults.
//========================================================================================
/*
Copyright (c) 2015-2021, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if DEBUG_OUTPUT
#define SERIAL_BAUD_RATE        115200           // *must* match this in serial monitor
#define MSECS_WAIT_FOR_USER     15000            // msecs to wait for serial monitor
#endif

#define DEFAULT_DEVICE_NAME     "PixelNutDevice"
#define MAXLEN_DEVICE_NAME      16              // maxlen for device name
#define PREFIX_DEVICE_NAME      "P!"            // for name to be recognized
#define PREFIX_LEN_DEVNAME      2               // length of this prefix

#if !defined(DEBUG_SIGNON)
#define DEBUG_SIGNON            DEFAULT_DEVICE_NAME
#endif

#if !defined(APIN_SEED)
#define APIN_SEED               A0              // default pin for seeding randomizer
#endif

#if !defined(PIXELS_APA) || PIXELS_APA
#if !defined(SPI_SETTINGS_FREQ)
#define SPI_SETTINGS_FREQ       4000000         // use fastest speed by default
#endif
#endif

#if !defined(PIXEL_OFFSET)
#define PIXEL_OFFSET            0               // start drawing at the first pixel
#endif
#if !defined(DIRECTION_UP)
#define DIRECTION_UP            true            // draw from start to end by default
#endif
#if !defined(MAX_BRIGHTNESS)
#define MAX_BRIGHTNESS          100             // default is to allow for maximum brightness
#endif
#if !defined(DELAY_OFFSET)
#define DELAY_OFFSET            0               // default is no additional delay
#endif
#if !defined(DELAY_RANGE)
#define DELAY_RANGE             60              // default for internal patterns
#endif

#if defined(BLE_BLUEFRUIT) && BLE_BLUEFRUIT
#if defined(__AVR_ATmega32U4__)
#define BLUEFRUIT_SPI_CS        8               // assign pins for Bluefruit Micro
#define BLUEFRUIT_SPI_IRQ       7               // ** hardcoded for this module **
#define BLUEFRUIT_SPI_RST       4
#else                                           // must override if change pins:
#define BLUEFRUIT_SPI_CS        10              // wire pins for Bluefruit SPI Friend
#define BLUEFRUIT_SPI_IRQ       9               //  (also connect MOSI, MISO, SCK pins
#define BLUEFRUIT_SPI_RST       8               //   to pins 11, 12, 13 respectively)
#endif
#define BLE_COMM                1               // using bluetooth BLE
#endif
#if defined(BLE_ESP32) && BLE_ESP32
#define BLE_COMM                1               // using bluetooth BLE
#endif
#if !defined(BLE_COMM)
#define BLE_COMM                0               // default is no bluetooth
#endif

#if defined(WIFI_SOFTAP) && WIFI_SOFTAP
#define WIFI_COMM               1               // using WiFi SoftAP mode
#endif
#if !defined(WIFI_COMM)
#define WIFI_COMM               0               // default is no wifi
#endif

#if (BLE_COMM || WIFI_COMM)
#define EXTERNAL_COMM           1               // external communications
#endif
#if !defined(EXTERNAL_COMM)
#define EXTERNAL_COMM           0               // define default value
#endif

#if !defined(STRANDS_MULTI)
#define STRANDS_MULTI           0               // have a single strand
#endif
#if !defined(SEGMENT_COUNT)
#define SEGMENT_COUNT           1               // have a single segment
#endif

#if !defined(NUM_PLUGIN_LAYERS)
#if (!STRANDS_MULTI && (SEGMENT_COUNT > 1))     // logical segments
#define NUM_PLUGIN_LAYERS       (4 + SEGMENT_COUNT)
#define NUM_PLUGIN_TRACKS       (3 + SEGMENT_COUNT)
#elif STRANDS_MULTI || ESP32     // or any 32-bit processor? FIXME
#define NUM_PLUGIN_LAYERS       16
#define NUM_PLUGIN_TRACKS       4
#else
#define NUM_PLUGIN_LAYERS       4
#define NUM_PLUGIN_TRACKS       3
#endif
#endif
#if !defined(STRLEN_PATTERNS)
#define STRLEN_PATTERNS         100             // must be long enough for patterns
#endif

#if (!STRANDS_MULTI && (SEGMENT_COUNT > 1))     // to support logical segments:
#define APPCMDS_EXTEND          1               // must extend AppCommands class
#define PIXENGINE_OVERRIDE      1               // must extend PixelNutEngine class
#endif

#if STRANDS_MULTI                               // multi physical strands
#define APPCMDS_EXTEND          1               // must have extended AppCommands
#define MAIN_OVERRIDE           1               // must replace both setup() and loop()
#define XMAIN_ENABLE            1               // with code here
#endif

#if !defined(MAIN_OVERRIDE)
#define MAIN_OVERRIDE           0
#endif
#if !defined(XMAIN_ENABLE)
#define XMAIN_ENABLE            0
#endif

#if XMAIN_ENABLE
#define SHOWPIX_OVERRIDE        1               // use different pixel displaying
#define PIXENGINE_OVERRIDE      1               // must have extended PixelNutEngine
#endif

#if !defined(PATTERN_OVERRIDE)
#define PATTERN_OVERRIDE        0               // not using configuration specific patterns
#endif
#if !defined(PLUGINS_OVERRIDE)
#define PLUGINS_OVERRIDE        0               // not using extended plugins by default
#endif

#if !defined(CUSTOM_PATTERNS)
// Use custom internal patterns if have plugins or no external client
#define CUSTOM_PATTERNS         ((PLUGINS_OVERRIDE || !EXTERNAL_COMM))
#endif

#if !defined(EXTERN_PATTERNS)
#define EXTERN_PATTERNS         EXTERNAL_COMM   // use external (client) patterns if possible
#endif

#if !defined(PLUGIN_PLASMA)
#define PLUGIN_PLASMA           PLUGINS_OVERRIDE // use plasma plugin if possible
#endif

#if !defined(PLUGIN_SPECTRA)
#define PLUGIN_SPECTRA          0               // only use this if specially requested
#endif
#if PLUGIN_SPECTRA
#define FREQ_FFT                1               // plugin uses frequency FFT
#endif

// Used to form patterns with multiple logical segments, with J,K,X,Y engine commands
#define MACRO_TO_STR(s) #s
#define MACRO_TO_MACRO(s) MACRO_TO_STR(s)
#define XY(x,y) "X" MACRO_TO_MACRO(x) " Y" MACRO_TO_MACRO(y) " "
#define JK(j,k) "J" MACRO_TO_MACRO(j) " K" MACRO_TO_MACRO(k) " "

//========================================================================================
