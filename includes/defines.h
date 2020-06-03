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

#if !defined(NEOPIXELS_OVERRIDE)
#define NEOPIXELS_OVERRIDE      0           // use neopixels by default
#endif

#if !defined(FEATURE_BITS)
#define FEATURE_BITS            0           // extended feature bits
#endif

#if !defined(BLUETOOTH_COMM)
#define  BLUETOOTH_COMM         0           // no bluetooth present
#elif BLUETOOTH_COMM
#define EXTERNAL_COMM           1           // external communications using bluetooth
#endif

#if !defined(WIFI_COMM)
#define  WIFI_COMM              0           // no wifi present
#elif WIFI_COMM
#define EXTERNAL_COMM           1           // external communications using wifi
#endif

#if !defined(EXTERNAL_COMM)
#define EXTERNAL_COMM           0           // no external client present
#endif

#if !defined(STRAND_COUNT)
#define STRAND_COUNT            1           // have a single strand
#endif

#if !defined(SEGMENT_COUNT)
#define SEGMENT_COUNT           1           // have a single segment
#endif

#if ((STRAND_COUNT > 1) && (SEGMENT_COUNT != STRAND_COUNT))
#error("Must be same number of segments as strands (if more than one strand)")
#endif

#if ((STRAND_COUNT <= 1) && (SEGMENT_COUNT > 1)) // to support logical segments:
#define PIXENGINE_OVERRIDE      1           // must extend engine class
#define APPCMDS_OVERRIDE        1           // must extend appcmds class
#define CUSTOM_OVERRIDE         1           // must extend custom class
#elif EXTERNAL_COMM
#define CUSTOM_OVERRIDE         1           // must extend for communications
#endif

#if !defined(CUSTOM_PATTERNS)
#define CUSTOM_PATTERNS         !EXTERNAL_COMM  // MUST use internal patterns if no external client
#endif

#if !defined(EXTERN_PATTERNS)
#define EXTERN_PATTERNS         EXTERNAL_COMM   // use external (client) patterns if possible
#endif

#if !defined(BASIC_PATTERNS)
#define BASIC_PATTERNS          0           // use extended patterns
#endif

#if !defined(CUSTOM_PLUGINS)
#define CUSTOM_PLUGINS          0           // no additional custom plugins
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

//========================================================================================
