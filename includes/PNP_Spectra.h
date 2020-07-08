//*********************************************************************************************
// This has been adapted from ???
// Modifications for use by PixelNut systems:
// Greg de Valois 2020
//*********************************************************************************************
// What Effect Does:
//
//    Fades to dark whatever has been previouly drawn on the strip, restarting the
//    process each time it is triggered.
//
// Calling trigger():
//
//    The trigger force is used to modify the time it takes to fade all pixels.
//    The larger the force, the longer it takes to fade to black.
//
// Calling nextstep():
//
//    Modifies the brightness of each pixel in the strip, then reduces the brightness
//    scaling by the amount calculated in trigger().
//
// Properties Used:
//
//    pixCount - current value is the counter for the number of pixels to clear and set.
//
// Properties Affected:
//
//    r,g,b - the raw pixel values
//
#if defined(PLUGIN_SPECTRA) && PLUGIN_SPECTRA

#if !defined(MATRIX_HEIGHT)
#define MATRIX_HEIGHT 0
#endif

#define SAMPLE_RATE_HZ          2000    // Sample rate of the audio in hertz
#define SPECTRUM_MIN_DB         40.0    // intensity (in decibels) that maps to low LED brightness
#define SPECTRUM_MAX_DB         80.0    // intensity (in decibels) that maps to high LED brightness

uint16_t* hueVals = NULL;
PixelNutHandle theHandle;
PixelNutSupport::DrawProps *thePdraw;

static void Spectra_SetPixel(int pos, float value)
{
  //pixelNutSupport.msgFormat(F("Spectra: pos=%d value=%.2f"), pos, value);

  thePdraw->degreeHue = hueVals[pos];
  thePdraw->pcentWhite = 0;

  #if (MATRIX_HEIGHT > 1)

  int pix = pos * MATRIX_HEIGHT;
  float last = (MATRIX_HEIGHT * value);
  int index = (int)last;
  value = last - (float)index;
  if (!(pos & 1)) index = MATRIX_HEIGHT - index;
  //pixelNutSupport.msgFormat(F("Spectra: pix=%d last=%.2f index=%d value=%.2f"), pix, last, index, value);

  for (int i = 0; i < index; ++i)
  {
    thePdraw->pcentBright = (pos & 1) ? 100 : 0;
    pixelNutSupport.makeColorVals(thePdraw);
    pixelNutSupport.setPixel(theHandle, pix+i, thePdraw->r, thePdraw->g, thePdraw->b);

    //pixelNutSupport.msgFormat(F("Spectra: %d+%d <= %d"), pix, i, thePdraw->pcentBright);
  }

  thePdraw->pcentBright = (value * 100);
  pixelNutSupport.makeColorVals(thePdraw);
  pixelNutSupport.setPixel(theHandle, pix+index, thePdraw->r, thePdraw->g, thePdraw->b);

  //pixelNutSupport.msgFormat(F("Spectra: %d+%d <= %d"), pix, index, thePdraw->pcentBright);

  for (int i = index+1; i < MATRIX_HEIGHT; ++i)
  {
    thePdraw->pcentBright = (pos & 1) ? 0 : 100;
    pixelNutSupport.makeColorVals(thePdraw);
    pixelNutSupport.setPixel(theHandle, pix+i, thePdraw->r, thePdraw->g, thePdraw->b);

    //pixelNutSupport.msgFormat(F("Spectra: %d+%d <= %d"), pix, i, thePdraw->pcentBright);
  }

  #else
  thePdraw->pcentBright = (value * 100);
  pixelNutSupport.makeColorVals(thePdraw);
  pixelNutSupport.setPixel(theHandle, pos, thePdraw->r, thePdraw->g, thePdraw->b);
  #endif
}

class PNP_Spectra : public PixelNutPlugin
{
public:

  byte gettype(void) const { return PLUGIN_TYPE_REDRAW | PLUGIN_TYPE_TRIGGER; };

  void begin(byte id, uint16_t pixlen)
  {
    #if (MATRIX_HEIGHT > 1)
    pixlen /= MATRIX_HEIGHT;
    #endif

    if (FreqFFT_Init(SAMPLE_RATE_HZ, pixlen))
    {
      pinMode(APIN_MICROPHONE, INPUT);

      hueVals = (uint16_t*)malloc(pixlen * sizeof(uint16_t));
      if (hueVals != NULL)
      {
        #if (MATRIX_HEIGHT > 1)
        for (int i = 0; i < pixlen; ++i) hueVals[i] = 0; // red
        #else
        // evenly spread hues across all pixels, starting with red
        // TODO: set manually for better color separation
        float inc = (float)MAX_DEGREES_HUE / (float)pixlen;
        float hue = 320.0;

        for (int i = 0; i < pixlen; ++i)
        {
          //pixelNutSupport.msgFormat(F("Spectra: %d) hue=%d"), i, (uint16_t)hue);
          hueVals[i] = (uint16_t)hue;
          hue += inc;
          if (hue >= MAX_DEGREES_HUE) hue = 0.0;
        }
        #endif
      }
      else FreqFFT_Fini();
    }
  }

  ~PNP_Spectra()
  {
    if (hueVals != NULL)
    {
      free(hueVals);
      hueVals = NULL;
      FreqFFT_Fini();
    }
  }

  void trigger(PixelNutHandle handle, PixelNutSupport::DrawProps *pdraw, short force)
  {
    if (hueVals != NULL)
    {
      theHandle = handle;
      thePdraw = pdraw;

      int min = SPECTRUM_MIN_DB;
      int max = SPECTRUM_MAX_DB;
      // TODO: use force to determine these
      FreqFFT_Begin(min, max);
    }
  }

  void nextstep(PixelNutHandle handle, PixelNutSupport::DrawProps *pdraw)
  {
    if (hueVals != NULL) FreqFFT_Next(Spectra_SetPixel);
  }
};

#endif // PLUGIN_SPECTRA
//*********************************************************************************************
