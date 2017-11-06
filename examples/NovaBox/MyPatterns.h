// PixelNut! Pattern Command Strings
// Customize your effect patterns here.

// adjust these if need more layers/tracks or longer patterns strings:
#define NUM_PLUGIN_LAYERS       4           // minimum number of plugins that to be combined at once
#define NUM_PLUGIN_TRACKS       3           // minimum layers that have unique properties (redraw pixels)
#define STRLEN_PATTERNS         110         // long enough for pattern strings (including ending 0 of string)
#define DELAY_RANGE             60          // range of delay values in msecs: match largest "D" command

//********************************************************************************************************************************************
// Each plugin ("E" command) must end with a "T" command to have it start drawing right away, else it must be externally Triggered.
// The final "G" is required at the end to activate the new effects.
//
// The "Q" command must be used with any track that you wish to have external control of the color hue ("H"), color whiteness
// ("W"), or pixel count ("C") properties, or to be externally Triggered (as opposed to using the "A" or "T" commands).
//********************************************************************************************************************************************

// Note: if the strings are aligned as shown, then to keep within an 109 character length they can't be longer than this ---------------------------------------^
// For example:                                   "-------------------------------------------------------------------------------------------------------------"

// Color hue changes "ripple" down the strip. The colors move through the spectrum, and appear stationary until Triggered.
// The Force applied changes the amount of color change per pixel. At maximum Force the entire spectrum is displayed again.
#define PATTERN_RAINBOW_RIPPLE                    "E2 D20 T E101 F1000 I T G"

// Colors hue changes occur at the head and get pushed down the strip. When the end is reached they start getting cleared,
// creating a "rolling" effect. Triggering restarts the effect, with the amount of Force determining how fast the colors change.
// At the maximum Force the entire spectrum is displayed again.
#define PATTERN_RAINBOW_ROLL                      "E1 D20 I T E101 F1000 I T G"

// This creates a "wave" effect (brightness that changes up and down) that move down the strip, in a single color. Triggering
// changes the frequency of the waves, with larger Forces making longer waves.
#define PATTERN_LIGHT_WAVES                       "E10 D60 T E101 T E120 F250 I T G"

// This has bright white twinkling over a soft blue background, like "stars in a blue sky". Triggering causes the background 
// brightness to swell up and down, with the amount of Force determining the speed of the swelling.
#define PATTERN_COLOR_TWINKLES                    "E0 B50 W20 H232 T E142 D10 F250 I E50 B80 W80 D10 T G"

// This also has bright white twinkling, but without a background. Instead, there are occasional comets that streak up and
// down the strip, and then disappear. One of the comets is red, is a fixed length, and appears randomly every 3-6 seconds.
// The other is orange, and appears only when Triggered, with the Force determining its length.
#define PATTERN_TWINKLE_COMETS                    "E50 B65 W80 H50 T E20 B90 C20 D30 F0 O3 T6 E20 U0 B90 H30 C25 D30 F0 I T E120 F1 I G"

// Comets pairs, one in either direction, both of which change color hue (but not whiteness) occasionaly. Trigging causes 
// new comets to be added (that keep going around), for a maximum of 12 per direction.
#define PATTERN_DUELING_COMETS                    "E20 W25 C25 D30 I T E101 F100 T E20 U0 W25 C25 D20 I T E101 F200 T G"

// Two scanners (blocks of same brightness pixels that move back and forth), with only the first one visible initially until
// a Trigger is applied. The first one changes colors on each change in direction. The second one (once Triggered) is deep
// purple, moves in the opposite direction, and periodically surges in speed.
#define PATTERN_DUELING_SCANNERS                  "E40 C25 D20 F300 T E111 A0 E40 U0 V1 H270 C5 D30 I E131 F1000 O5 T5 G"

// Evenly spaced pixels ("spokes") move together around and around the strip, creating a "Ferris Wheel" effect. The spokes 
// periodically change to random colors. Triggering toggles the direction, and determines how many spokes there are (larger
// Forces for more spokes).
#define PATTERN_FERRIS_WHEEL                      "E30 C20 D60 T E160 I E120 F250 I E111 F O3 T7 G"

// The background is white-ish noise (randomly sets pixels with random brightness of unsaturated colors, slowly and
// continuously expand and contract, with the Force used when Triggering determining the extent of the expansion.
#define PATTERN_EXPANDING_NOISE                   "E52 C25 W65 D20 T E150 D60 T E120 F1000 I T G"

// Random colored blinking that periodically surge in the rate of blinking. Triggering changes the frequency of the blinking,
// with larger Forces causing faster blinking surges.
#define PATTERN_BLINK_SURGES                      "E51 C10 D60 T E112 T E131 F1000 I T G"

// All pixels swell up and down in brightness, with random color hue and whiteness changes every 10 seconds. Triggering 
// changes the pace of the swelling, with larger Forces causing faster swelling.
#define PATTERN_BRIGHT_SWELLS                     "E0 B80 T E111 F O10 T10 E142 F250 I T G"

// All pixels move through color hue and whiteness transitions that are slow and smooth. A new color is chosen every time
// the previous target color has been reached, or when Triggered, with the Force determining how large the reoccurring color
// changes are. The time it takes to reach a new color is proportional to the size of the color change.
#define PATTERN_COLOR_SMOOTH                      "E0 H30 D30 T E110 F600 I T E111 A1 G"

// Combination of a purple scanner over a green-ish twinkling background, with a red comet that is fired off every time
// the scanner bounces off the end of the strip, or when Triggered, which disappear off the end.
#define PATTERN_MASHUP                            "E50 V1 B65 W30 H100 T E40 H270 C10 D50 T E20 D15 C20 A1 F0 I T G"

static PROGMEM const char pattern_1[]  = PATTERN_RAINBOW_RIPPLE;
static PROGMEM const char pattern_2[]  = PATTERN_RAINBOW_ROLL;
static PROGMEM const char pattern_3[]  = PATTERN_LIGHT_WAVES;
static PROGMEM const char pattern_4[]  = PATTERN_COLOR_TWINKLES;
static PROGMEM const char pattern_5[]  = PATTERN_TWINKLE_COMETS;
static PROGMEM const char pattern_6[]  = PATTERN_DUELING_COMETS;
static PROGMEM const char pattern_7[]  = PATTERN_DUELING_SCANNERS;
static PROGMEM const char pattern_8[]  = PATTERN_FERRIS_WHEEL;
static PROGMEM const char pattern_9[]  = PATTERN_EXPANDING_NOISE;
static PROGMEM const char pattern_10[] = PATTERN_BLINK_SURGES;
static PROGMEM const char pattern_11[] = PATTERN_BRIGHT_SWELLS;
static PROGMEM const char pattern_12[] = PATTERN_COLOR_SMOOTH;
static PROGMEM const char pattern_13[] = PATTERN_MASHUP;

static const char* const customPatterns[] =
{
  pattern_1,
  pattern_2,
  pattern_3,
  pattern_4,
  pattern_5,
  pattern_6,
  pattern_7,
  pattern_8,
  pattern_9,
  pattern_10,
  pattern_11,
  pattern_12,
  pattern_13,

  0 // NULL terminator
};

