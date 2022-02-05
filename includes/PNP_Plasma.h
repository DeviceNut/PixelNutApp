//*********************************************************************************************
// This has been adapted from "neopixelplasma.cpp".
// Copyright (C) 2013 John Ericksen (GNU license)
// Modifications for use by PixelNut systems:
// Greg de Valois 2017
//
// Maps points along Lissajious curves to 2 dimensions.
// http://en.wikipedia.org/wiki/Lissajous_curve
//*********************************************************************************************
// What Effect Does:
//
//    Fades to dark whatever has been previouly drawn on the strip, restarting the
//    process each time it is triggered.
//
// Calling trigger():
//
//    Not instantiated.
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
#if defined(PLUGIN_PLASMA) && PLUGIN_PLASMA

#define COLOR_STRETCH 0.5    // larger numbers for tighter color bands
#define MIN_PHASE_INC 0.0001 // larger numbers for faster points
#define MAX_PHASE_INC 0.04

struct Point
{
  float x;
  float y;
};

class PNP_Plasma : public PixelNutPlugin
{
public:
  byte gettype(void) const { return PLUGIN_TYPE_REDRAW; };

  void begin(byte id, uint16_t pixlen)
  {
    pixLength = pixlen;
    numrows = numcols = (uint16_t)sqrt(pixlen);
    while ((numrows * numcols) < pixlen) ++numcols;
    if (numcols > numrows) --numcols; // back off one if did increment
    endcol = numcols + (pixlen - (numcols * numrows));
    //DBGOUT((F("Plasma: pixs=%d rows=%d cols=%d endcol=%d"), pixlen, numrows, numcols, endcol));
  }

  void nextstep(PixelNutHandle handle, PixelNutSupport::DrawProps *pdraw)
  {
    uint16_t pcent = ((pdraw->pixCount * MAX_PERCENTAGE) / pixLength / 3);
    float pinc = (pcent * (MAX_PHASE_INC - MIN_PHASE_INC))/MAX_PERCENTAGE + MIN_PHASE_INC;
    phase += pinc;
    //DBGOUT((F("Plasma: pcent=%d phase=%.4f pinc=%.4f"), pcent, phase, pinc));

    Point p1 = { static_cast<float>(((sin(phase * 1.000)+1.0)/2.0)*(numrows-1)), static_cast<float>(((sin(phase * 1.310)+1.0)/2.0)*(numcols-1)) };
    Point p2 = { static_cast<float>(((sin(phase * 1.770)+1.0)/2.0)*(numrows-1)), static_cast<float>(((sin(phase * 2.865)+1.0)/2.0)*(numcols-1)) };
    Point p3 = { static_cast<float>(((sin(phase * 0.250)+1.0)/2.0)*(numrows-1)), static_cast<float>(((sin(phase * 0.750)+1.0)/2.0)*(numcols-1)) };

    //DBGOUT((F("Plasma: P1: %d.%d"), (int)p1.x, (int)p1.y));
    //DBGOUT((F("Plasma: P2: %d.%d"), (int)p2.x, (int)p2.y));
    //DBGOUT((F("Plasma: P3: %d.%d"), (int)p3.x, (int)p3.y));

    for (uint16_t row = 0; row < numrows; ++row)
    {
      float row_f = float(row);
      
      for (uint16_t col = 0; col < endcol; ++col)
      {
        float col_f = float(col);

        // Calculate the distance between this LED, and p1.
        Point dist1 = { col_f - p1.x, row_f - p1.y };  // The vector from p1 to this LED.
        float distance1 = sqrt(dist1.x * dist1.x + dist1.y * dist1.y);

        // Calculate the distance between this LED, and p2.
        Point dist2 = { col_f - p2.x, row_f - p2.y };  // The vector from p2 to this LED.
        float distance2 = sqrt(dist2.x * dist2.x + dist2.y * dist2.y);

        // Calculate the distance between this LED, and p3.
        Point dist3 = { col_f - p3.x, row_f - p3.y };  // The vector from p3 to this LED.
        float distance3 = sqrt(dist3.x * dist3.x + dist3.y * dist3.y);

        //DBGOUT((F("Plasma: distances(%.4f %.4f %.4f)"), distance1, distance2, distance3));

        // Warp the distance with a sin() function. As the distance value increases, the LEDs will get light,dark,light,dark...
        float color_1 = distance1;  // range: 0.0...1.0
        float color_2 = distance2;
        float color_3 = distance3;
        float color_4 = (sin(distance1 * distance2 * COLOR_STRETCH)) + 1.0;
        //DBGOUT((F("Plasma: colors(%.4f %.4f %.4f %.4f)"), color_1, color_2, color_3, color_4));

        // Square the color_f value to weight it towards 0. The image will be darker and have higher contrast.
        color_1 *= color_1 * color_4;
        color_2 *= color_2 * color_4;
        color_3 *= color_3 * color_4;
        color_4 *= color_4;
        //DBGOUT((F("Plasma: colors(%.4f %.4f %.4f %.4f)"), color_1, color_2, color_3, color_4));

        //DBGOUT((F("Plasma: pos=%d color=%d.%d.%d"), col + (numcols * row), (byte)color_1, (byte)color_2, (byte)color_3));
        pixelNutSupport.setPixel(handle, col + (numcols * row), (byte)color_1, (byte)color_2, (byte)color_3);
      }
    }
  }

private:
  uint16_t pixLength;
  uint16_t numrows, numcols, endcol;
  float phase = 0.0;
};

#endif // PLUGIN_PLASMA
//*********************************************************************************************
