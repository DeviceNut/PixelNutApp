// PixelNutApp Frequency FFT Routines
//========================================================================================
/*
Copyright (c) 2015-2020, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/
#if defined(FREQ_FFT) && FREQ_FFT

#define ARM_MATH_CM4
#include <arm_math.h>

#define ANALOG_READ_RESOLUTION  10      // Bits of resolution for the ADC
#define ANALOG_READ_AVERAGING   16      // Number of samples to average with each ADC reading
#define FFT_SIZE                64      // Size of the FFT.  Realistically can only be at most 256 
                                        // without running out of memory for buffers and other state

typedef void (*FreqFFT_SetPos_CB)(int pos, float value);

static IntervalTimer samplingTimer;
static float samples[FFT_SIZE*2];
static int sampleCounter = 0;

static float magnitudes[FFT_SIZE];
static float* freqWindow = NULL;
static int freqCount;

static int minDB, maxDB;
static int sampleRate;

static void FreqFFT_GetData_CB()
{
  int data = analogRead(APIN_MICROPHONE);
  //DBGOUT((F("FreqFFT: %d) data=%d"), sampleCounter, data));

  samples[sampleCounter++] = (float32_t)data;
  // Complex FFT functions require a coefficient for the imaginary part of the input.
  // Since we only have real data, set this coefficient to zero.
  samples[sampleCounter++] = 0.0;

  // stop after the buffer is filled
  if (sampleCounter >= FFT_SIZE*2) samplingTimer.end();
}

// Reset sample buffer position and start callback at necessary rate
static void samplingBegin(void)
{
  sampleCounter = 0;
  samplingTimer.begin(FreqFFT_GetData_CB, 1000000/sampleRate);
}

bool FreqFFT_Init(int rate, uint16_t count)
{
  analogReadResolution(ANALOG_READ_RESOLUTION);
  analogReadAveraging(ANALOG_READ_AVERAGING);

  sampleRate = rate;
  freqCount = count;

  freqWindow = (float*)malloc(sizeof(float) * freqCount+1);
  if (freqWindow == NULL) return false;

  // Set the frequency window values by evenly dividing the
  // possible frequency spectra across the number of slots
  float windowSize = (sampleRate / 2.0) / float(freqCount);

  for (int i = 0; i <= freqCount; ++i)
    freqWindow[i] = i * windowSize;

  return true;
}

void FreqFFT_Fini(void)
{
  if (freqWindow != NULL)
  {
    free(freqWindow);
    freqWindow = NULL;
  }
}

void FreqFFT_Begin(int min, int max)
{
  minDB = min;
  maxDB = max;
  samplingBegin();
}

// Compute the average magnitude of a target frequency window
static float windowMean(float* magnitudes, int lowBin, int highBin)
{
    float value = 0.0;

    // first bin is skipped because it represents average signal power
    for (int i = 1; i < FFT_SIZE/2; ++i)
    {
      if (lowBin <= i) value += magnitudes[i];
      if (i > highBin) break;
    }

    return value / (highBin - lowBin + 1);
}

// Convert a frequency to the appropriate FFT bin it falls within
static int inline frequencyToBin(float frequency)
{
  float binFrequency = float(sampleRate) / float(FFT_SIZE);
  return int(frequency / binFrequency);
}

void FreqFFT_Next(FreqFFT_SetPos_CB valueCB)
{
  // calculate FFT once a full sample is available
  if (sampleCounter >= FFT_SIZE*2)
  {
    // run FFT on sample data
    arm_cfft_radix4_instance_f32 fft_inst;
    arm_cfft_radix4_init_f32(&fft_inst, FFT_SIZE, 0, 1);
    arm_cfft_radix4_f32(&fft_inst, samples);

    // calculate magnitude of complex numbers output by the FFT
    arm_cmplx_mag_f32(samples, magnitudes, FFT_SIZE);
  
    // calculate intensity in associated frequency window
    float intensity;
    for (int i = 0; i < freqCount; ++i)
    {
      intensity = windowMean(magnitudes, 
                    frequencyToBin(freqWindow[i]),
                    frequencyToBin(freqWindow[i+1]));

      // convert intensity to decibels
      intensity = 20.0*log10(intensity);

      // scale the intensity and clamp between 0.05 and 1.0
      intensity -= minDB;
      intensity = intensity < 0.0 ? 0.0 : intensity;
      intensity /= (maxDB - minDB);
      intensity = intensity > 1.0  ? 1.0  : intensity;
      intensity = intensity < 0.05 ? 0.05 : intensity;

      (*valueCB)(i, intensity);
    }

    samplingBegin();
  }
}

#endif // FREQ_FFT
//========================================================================================
