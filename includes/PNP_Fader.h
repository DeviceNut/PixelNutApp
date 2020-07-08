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
#if defined(PLUGIN_FADER) && PLUGIN_FADER

class PNP_ForceFade : public PixelNutPlugin
{
public:
  byte gettype(void) const
  {
    return PLUGIN_TYPE_POSTDRAW | PLUGIN_TYPE_TRIGGER;
  };

  void begin(byte id, uint16_t pixlen)
  {
    pixLength = pixlen;
  }

  void trigger(PixelNutHandle handle, PixelNutSupport::DrawProps *pdraw, short force)
  {
    force = abs(force);
    if (force == MAX_FORCE_VALUE) --force; // insure step is not 0

    scale_step = (((float)MAX_FORCE_VALUE - force) / ((float)MAX_FORCE_VALUE * 20)); // at least 20 steps
    scale_factor = 1.0; // start with full scaling

    //pixelNutSupport.msgFormat(F("ForceFade: force=%d steps=%d"), force, (int)(1/scale_step));
  }

  void nextstep(PixelNutHandle handle, PixelNutSupport::DrawProps *pdraw)
  {
    //pixelNutSupport.msgFormat(F("ForceFade: scale=%d%% step=%d%%"), (int)(scale_factor*100), (int)(scale_step*100));

    for (uint16_t i = 0; i < pixLength; ++i)
      pixelNutSupport.setPixel(handle, i, scale_factor);

    if (pdraw != NULL)
    {
      if ( scale_factor > scale_step)
           scale_factor -= scale_step;
      else scale_factor = 0.0;
    }
  }

private:
  uint16_t pixLength;   // how many pixels to draw
  float scale_factor;   // scaling factor to fade with
  float scale_step;     // amount to reduce by each step
};

#endif // PLUGIN_FADER
//*********************************************************************************************
