// PixelNutApp Global Definitions
// Insure certain required definitions are set to defaults.
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if !defined(FEATURE_BITS)
#define FEATURE_BITS            0           // extended feature bits
#endif

#if BLUETOOTH_COMM
#define EXTERNAL_COMM           1           // external communications uses bluetooth
#include <SPI.h>
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

#if ((STRAND_COUNT <= 1) && (SEGMENT_COUNT > 1)) // in order to support:
#define PIXENGINE_OVERRIDE      1           // must extend engine class
#define APPCMDS_OVERRIDE        1           // must extend appcmds class
#define CUSTOM_OVERRIDE         1           // must extend custom class

#elif BLUETOOTH_COMM
#define CUSTOM_OVERRIDE         1           // must extend for bluetooth
#endif

#if !defined(EXTERNAL_COMM)
#define EXTERNAL_COMM           0
#endif

#if !defined(CUSTOM_PATTERNS) // use internal patterns if no external client
#define CUSTOM_PATTERNS         !EXTERNAL_COMM
#endif

#if !defined(BASIC_PATTERNS)
#define BASIC_PATTERNS          0           // patterns/plugins not restricted
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
