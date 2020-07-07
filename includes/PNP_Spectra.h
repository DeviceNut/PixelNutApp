#if PLUGIN_SPECTRA

#define ARM_MATH_CM4
#include <arm_math.h>

#define SAMPLE_RATE_HZ          500     // Sample rate of the audio in hertz
#define ANALOG_READ_RESOLUTION  10      // Bits of resolution for the ADC
#define ANALOG_READ_AVERAGING   16      // Number of samples to average with each ADC reading
#define FFT_SIZE                64      // Size of the FFT.  Realistically can only be at most 256 
                                        // without running out of memory for buffers and other state
#define SPECTRUM_MIN_DB         40.0    // intensity (in decibels) that maps to low LED brightness
#define SPECTRUM_MAX_DB         80.0    // intensity (in decibels) that maps to high LED brightness

IntervalTimer samplingTimer;
float samples[FFT_SIZE*2];
int sampleCounter;

class PNP_Spectra : public PixelNutPlugin
{
public:
  byte gettype(void) const { return PLUGIN_TYPE_REDRAW; };

  void begin(byte id, uint16_t pixlen)
  {
    pixLength = pixlen; // total number of pixels

    pinMode(APIN_MICROPHONE, INPUT);
    analogReadResolution(ANALOG_READ_RESOLUTION);
    analogReadAveraging(ANALOG_READ_AVERAGING);

    spectrumSetup();
  }

  void trigger(PixelNutHandle handle, PixelNutSupport::DrawProps *pdraw, short force)
  {
    samplingBegin();
  }

  void nextstep(PixelNutHandle handle, PixelNutSupport::DrawProps *pdraw)
  {
    // Calculate FFT if a full sample is available
    if (sampleCounter >= FFT_SIZE*2)
    {
      // Run FFT on sample data.
      arm_cfft_radix4_instance_f32 fft_inst;
      arm_cfft_radix4_init_f32(&fft_inst, FFT_SIZE, 0, 1);
      arm_cfft_radix4_f32(&fft_inst, samples);

      // Calculate magnitude of complex numbers output by the FFT.
      arm_cmplx_mag_f32(samples, magnitudes, FFT_SIZE);
    
      // Update each LED based on the intensity of the
      // audio in the associated frequency window
      float intensity, otherMean;
      for (int i = 0; i < pixLength; ++i)
      {
        windowMean(magnitudes, 
                  frequencyToBin(frequencyWindow[i]),
                  frequencyToBin(frequencyWindow[i+1]),
                  &intensity,
                  &otherMean);

        // Convert intensity to decibels
        intensity = 20.0*log10(intensity);

        // Scale the intensity and clamp between 0.05 and 1.0.
        intensity -= SPECTRUM_MIN_DB;
        intensity = intensity < 0.0 ? 0.0 : intensity;
        intensity /= (SPECTRUM_MAX_DB-SPECTRUM_MIN_DB);
        intensity = intensity > 1.0  ? 1.0  : intensity;
        intensity = intensity < 0.05 ? 0.05 : intensity;

        pdraw->degreeHue = hues[i];
        pdraw->pcentWhite = 0;
        pdraw->pcentBright = (intensity * 100);

        pixelNutSupport.makeColorVals(pdraw);
        pixelNutSupport.setPixel(handle, i, pdraw->r, pdraw->g, pdraw->b);

        pixelNutSupport.msgFormat(F("Spectra: %d) H=%d W=%d B=%d"), i,
            pdraw->degreeHue, pdraw->pcentWhite, pdraw->pcentBright);
      }

      samplingBegin();
    }
  }

private:
  uint16_t pixLength;

  float magnitudes[FFT_SIZE];
  float frequencyWindow[PIXEL_COUNT+1];
  float hues[PIXEL_COUNT];

  // Compute the average magnitude of a target frequency window vs. all other frequencies
  void windowMean(float* magnitudes, int lowBin, int highBin, float* windowMean, float* otherMean)
  {
      *windowMean = 0;
      *otherMean = 0;
      // Notice the first magnitude bin is skipped because it represents the average power of the signal
      for (int i = 1; i < FFT_SIZE/2; ++i)
      {
        if (i >= lowBin && i <= highBin)
             *windowMean += magnitudes[i];
        else *otherMean  += magnitudes[i];
      }
      *windowMean /= (highBin - lowBin) + 1;
      *otherMean /= (FFT_SIZE/2 - (highBin - lowBin));
  }
  
  // Convert a frequency to the appropriate FFT bin it will fall within
  int frequencyToBin(float frequency)
  {
    float binFrequency = float(SAMPLE_RATE_HZ) / float(FFT_SIZE);
    return int(frequency / binFrequency);
  }

  void spectrumSetup()
  {
    // Set the frequency window values by evenly dividing the
    // possible frequency spectrum across the number of pixels
    float windowSize = (SAMPLE_RATE_HZ / 2.0) / float(pixLength);
    for (int i = 0; i < pixLength+1; ++i)
      frequencyWindow[i] = i*windowSize;

    // Evenly spread hues across all pixels
    for (int i = 0; i < pixLength; ++i)
      hues[i] = 360.0*(float(i)/float(pixLength-1));
  }

  static void samplingCallback()
  {
    // Read from the ADC and store the sample data
    samples[sampleCounter] = (float32_t)analogRead(APIN_MICROPHONE);

    // Complex FFT functions require a coefficient for the imaginary part of the input.
    // Since we only have real data, set this coefficient to zero.
    samples[sampleCounter+1] = 0.0;

    // Update sample buffer position and stop after the buffer is filled
    sampleCounter += 2;

    if (sampleCounter >= FFT_SIZE*2) samplingTimer.end();
  }
  
  void samplingBegin()
  {
    // Reset sample buffer position and start callback at necessary rate.
    sampleCounter = 0;
    samplingTimer.begin(samplingCallback, 1000000/SAMPLE_RATE_HZ);
  }
};

#endif // PLUGIN_SPECTRA
