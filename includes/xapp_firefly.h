//*********************************************************************************************
// Custom Setup & Loop routines for the FireFly configuration.
//*********************************************************************************************
/*
Copyright (c) 2015-2020, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if defined(XAPP_FIREFLY) && XAPP_FIREFLY

// Patterns used:
// 1) when at rest - purple swelling, with triggered orange comets
// 2) X direction  - waves of changing colors, triggering surges speed of wave
// 3) Y direction  - magenta twinkles over dim yellow background, triggering surges background brightness
// 4) Z direction  - scanner (initially yellow), triggering changes the color and surges the scanner speed
// 5) XY direction - yellow twinkles over blue wave background, with triggered red comets that change direction
// 6) YZ direction - ferris wheel, triggering changes color, direction, and number of spokes
// 7) XZ direction - expanding pink blinky window, triggering surges rate of expansion
// 8) no clear dir - multicolor random blinking, triggering surges speed of blinking

static PROGMEM const char pattern_1[] = "E0 B50 H270 T E142 F300 T E20 V1 H30 C20 D20 F0 I T G";
static PROGMEM const char pattern_2[] = "E10 C50 D40 T E101 T E131 F100 I T G";
static PROGMEM const char pattern_3[] = "E0 B20 H60 T E141 F1 I T E50 B70 H300 C50 D10 T G";
static PROGMEM const char pattern_4[] = "E40 H60 C20 D20 T E100 I T E131 F1000 I G";
static PROGMEM const char pattern_5[] = "E10 B40 H230 C20 D50 T E50 V1 B50 H60 C50 T E122 F100 T E20 C5 F0 I T E160 I G";
static PROGMEM const char pattern_6[] = "E30 C20 D60 T E160 I E120 F250 I E101 I G";
static PROGMEM const char pattern_7[] = "E52 H350 W45 C100 D30 T E150 T E131 F1000 I T G";
static PROGMEM const char pattern_8[] = "E51 C10 D60 T E112 T E131 F1000 I T G";

const char* const customPatterns[] =
{
  pattern_1,
  pattern_2,
  pattern_3,
  pattern_4,
  pattern_5,
  pattern_6,
  pattern_7,
  pattern_8,
  0
};

#define USE_LIS3DH            1             // 1 to use LIS3DH board
                                            // else read from pins
#if USE_LIS3DH
#include <Adafruit_LIS3DH.h>
Adafruit_LIS3DH lis3dh = Adafruit_LIS3DH();
#endif

// enable for selected debug output (DEBUG_OUTPUT must be set)
#define DEBUG_DATA_RAW        0             // prints raw input values
#define DEBUG_DATA_BUFF       0             // prints values saved in buffer
#define DEBUG_POINTS_STOP     0             // prints points and stops
#define DEBUG_CALIBRATION     0             // prints calibration reads
#define DEBUG_CALC_DATA       0             // prints calculated data
#define DEBUG_CAPTURE_DATA    0             // prints capture data
#define DEBUG_CAPTURE_DONE    0             // prints capture events
#define DEBUG_TRIGGER_DATA    0             // prints trigger data
#define DEBUG_TRIGGER_ACTION  0             // prints trigger action
#define DEBUG_STATE_CHANGE    0             // prints state change events

#define DEFAULT_FADER_ON      1             // 0 to disable fade, play forever

// calibration start delay is necessary if accelerometer needs to warmup
#define MSECS_CALIB_START     0             // msecs to delay before start
#define MSECS_CALIB_LOOP      1             // msecs to delay in each loop
#define COUNT_CALIB_LOOPS     300           // number of calibration loops

// accelerometer parameters
#define NOISE_CUTOFF          20            // min value not noise
#define MAX_MAG_VALUE         800           // largest magnitude

// determines how often main loop gets executed, and affects everything
// Note: updating pixels can take even longer, so this is a minimum
#define MSECS_LOOPTIME        10            // msecs to wait between each loop

// pixels do not fade well after minimal brightness and lose color
#define MAX_BRIGHT            100           // maximum brightness value
#define MIN_BRIGHT            10            // minimum allowed value

// tuning parameters for trigger/capture: adjust for desired behavior
#define MSECS_TO_TRIGGER      500           // msecs of elevated mag to trigger
#define MSECS_TRIG_REARM      1500          // msecs to rearm after last trigger
#define MSECS_TO_CAPTURE      1200          // msecs of elevated mag to capture
#define MSECS_BETWEEN_CALCS   100           // msecs between trigger/capture calculations

#define MIN_MAG_TRIGGER       160           // min magnitude value/count to trigger
#define MIN_MAG_CAPTURE       270           // min magnitude value to capture

// this determines which axes are to be a contributing factor in pattern selection
#define MIN_AVEDIFF_TOWIN     70            // min averaged value difference

// turning parameters for state transitions
#define MSECS_MIN_BREATHING   2000          // min msecs breathing before allow capture
#define MSECS_MIN_PLAYING     1500          // min msecs playing before start fade
#define MSECS_MIN_DARKNESS    250           // min msecs in dark before restart breathing

#define MSECS_DIMMER_MIN      4000          // min msecs to dim: 4 seconds
#define MSECS_DIMMER_MAX      10000         // max msecs to dim: 10 seconds
#define MSECS_DIMMER(m)       (MSECS_DIMMER_MIN + \
                              (int)(((float)(m-MIN_MAG_CAPTURE)/(MAX_MAG_VALUE-MIN_MAG_CAPTURE)) *\
                                            2*(MSECS_DIMMER_MAX-MSECS_DIMMER_MIN)))
#define STEPS_DIMMER(m)       ((MSECS_DIMMER(m)/MSECS_LOOPTIME)/(MAX_BRIGHT-MIN_BRIGHT))

#define CAPTURE_WAIT_FACTOR   6            // factor to increase msecs of capture for play time
#define MSECS_TOPLAY(msecs)   (msecs * CAPTURE_WAIT_FACTOR) // 1 sec of capture = 10 secs of playing

#define STEPS_FINALE_BRIGHT   1             // loops per brightness adjustment in finale

// number of data points used for trigger/capture event calculations
#define DATACOUNT_TRIGGER   (MSECS_TO_TRIGGER/MSECS_LOOPTIME)
#define DATACOUNT_CAPTURE   (MSECS_TO_CAPTURE/MSECS_LOOPTIME)

// size of data buffer has to be larger than that needed for an event
#define MAXNUM_DATA_VALUES  (DATACOUNT_CAPTURE+1)

static enum state
{
  STATE_CAPTURE = 0,    // capture user inputs
  STATE_FADING,         // playing pattern with fade
  STATE_FINALE,         // final brightness flourish
  STATE_RESTART,        // pause in dark before restart
}
theState = STATE_CAPTURE;

typedef struct
{
  int32_t x, y, z, m;   // X,Y,Z and calculated magnitude values
}
DataPoint;

static uint32_t loopTime;               // previous time of loop execution
static uint32_t stateTime;              // previous time state handled
static uint32_t triggerTime;            // time next trigger is allowed
static uint32_t captureTime;            // time next capture is allowed
static uint32_t faderTime;              // time when start fading pattern

static DataPoint calibVals;             // used in calibrating input values
static DataPoint dataBuffers[MAXNUM_DATA_VALUES]; // sliding window buffer

static uint16_t dataCountTrigger = 0;   // current number of points for trigger
static uint16_t dataCountCapture = 0;   // current number of points for capture
static int16_t dataIndexCapture = 0;    // index of start of capture data
static int16_t dataIndexTrigger = 0;    // index of start of trigger data
static int16_t dataIndexWrite = 0;      // index of current write point

static DataPoint calcVals;              // calculated values from buffer data
static bool captureStart = false;       // true once capture mode started
static int captureCount;                // number of points in capture event
static int captureMsecs;                // msecs elapsed during capture event
static DataPoint captureMax;            // point at maximum capture magnitude

static bool faderEnabled = DEFAULT_FADER_ON; // disabled to play forever
static bool faderWaiting;               // true when captured but not fading

static byte maxBright = MAX_BRIGHT;     // current brightness setting
static int stepsFadeAdjust;             // number of steps between brightness adjustment
static int stepsFadeCount;              // current countdown for decrementing brightness

#if defined(DPIN_FADER_BUTTON)
UIDeviceButton button_fader(DPIN_FADER_BUTTON);
#endif

#if DEBUG_POINTS_STOP
static void PrintDataAndStop(void)
{
  int count = 0;

  for (int i = dataIndexCapture; i != dataIndexWrite; ++count)
  {
    DBGOUT((F("%d,%d,%d,%d"), dataBuffers[i].x, dataBuffers[i].y, dataBuffers[i].z, dataBuffers[i].m));

    if (++i >= MAXNUM_DATA_VALUES) i = 0;
  }
  DBGOUT((F("Count=%d"), count));

  while(1); // stop here
}
#endif

// Initialize use of particular device
static void AccelerometerInit(void)
{
  #if USE_LIS3DH
  if (!lis3dh.begin(0x18)) ErrorHandler(2, 1, true);
  lis3dh.setRange(LIS3DH_RANGE_2_G);
  #else
  pinMode(ACC_APIN_XOUT, INPUT);
  pinMode(ACC_APIN_YOUT, INPUT);
  pinMode(ACC_APIN_ZOUT, INPUT);
  #endif
}

// Read values from device with 10 bits of resolution
static void ReadInputs(DataPoint* p)
{
  #if USE_LIS3DH
  lis3dh.read(); // has 16 bits of resolution
  p->x = lis3dh.x >> 6;
  p->y = lis3dh.y >> 6;
  p->z = lis3dh.z >> 6;
  #else
  p->x = analogRead(ACC_APIN_XOUT);
  p->y = analogRead(ACC_APIN_YOUT);
  p->z = analogRead(ACC_APIN_ZOUT);
  #endif
}

// Averages values used for input calibration
static void CalibrateValues(void)
{
  if (MSECS_CALIB_START > 0)
  {
    DBGOUT((F("Calibration settle time: %d msecs"), MSECS_CALIB_START));
    delay(MSECS_CALIB_START);
  }

  DataPoint vals, adds;
  adds.x = adds.y = adds.z = 0;

  DBGOUT((F("Calibrating: loops=%d delay=%d msecs"), COUNT_CALIB_LOOPS, MSECS_CALIB_LOOP));
  for (int count = 0; count < COUNT_CALIB_LOOPS; ++count)
  {
    delay(MSECS_CALIB_LOOP);

    ReadInputs(&vals);

    #if DEBUG_CALIBRATION
    DBGOUT((F("%d) X=%-3d Y=%-3d Z=%-3d"), count, vals.x, vals.y, vals.z));
    #endif

    adds.x += vals.x;
    adds.y += vals.y;
    adds.z += vals.z;
  }

  calibVals.x = (adds.x / COUNT_CALIB_LOOPS);
  calibVals.y = (adds.y / COUNT_CALIB_LOOPS);
  calibVals.z = (adds.z / COUNT_CALIB_LOOPS);

  DBGOUT((F("Calibration: X=%d Y=%d Z=%d"), calibVals.x, calibVals.y, calibVals.z));
}

// Initializes the data buffers indices
static void ClearBuffers(void)
{
  dataCountTrigger = dataCountCapture = 0;
  dataIndexTrigger = dataIndexCapture = dataIndexWrite = 0;
}

// Adds one point to the X,Y,Z,M data buffers
static void AddToBuffer(DataPoint* p)
{
  dataBuffers[dataIndexWrite].x = p->x;
  dataBuffers[dataIndexWrite].y = p->y;
  dataBuffers[dataIndexWrite].z = p->z;
  dataBuffers[dataIndexWrite].m = p->m;

  #if DEBUG_DATA_BUFF
  DBGOUT((F("%3d) X=%-3d Y=%-3d Z=%-3d M=%-3d (%d.%d-%d.%d)"),
          dataIndexWrite, p->x, p->y, p->z, p->m,
          dataIndexTrigger, dataCountTrigger,
          dataIndexCapture, dataCountCapture));
  #endif

  if (++dataIndexWrite >= MAXNUM_DATA_VALUES)
    dataIndexWrite = 0;

  if (dataCountTrigger >= DATACOUNT_TRIGGER)
  {
    if (++dataIndexTrigger >= MAXNUM_DATA_VALUES)
      dataIndexTrigger = 0;
  }
  else dataCountTrigger++;

  if (dataCountCapture >= DATACOUNT_CAPTURE)
  {
    if (++dataIndexCapture >= MAXNUM_DATA_VALUES)
      dataIndexCapture = 0;
  }
  else dataCountCapture++;
}

// Produces and stores one data point, that accounts for the
// initial calibration values and filters out low-freq noise
static void ProcessInputs(void)
{
  DataPoint inputs, newvals;
  ReadInputs(&inputs);

  #if DEBUG_DATA_RAW
  DBGOUT((F("x=%d y=%d z=%d"), inputs.x, inputs.y, inputs.z));
  #endif

  newvals.x = abs(inputs.x - calibVals.x);
  newvals.y = abs(inputs.y - calibVals.y);
  newvals.z = abs(inputs.z - calibVals.z);

  if (newvals.x < NOISE_CUTOFF) newvals.x = 0;
  if (newvals.y < NOISE_CUTOFF) newvals.y = 0;
  if (newvals.z < NOISE_CUTOFF) newvals.z = 0;

  newvals.m = sqrt((newvals.x*newvals.x) + (newvals.y*newvals.y) + (newvals.z*newvals.z));

  AddToBuffer(&newvals);
}

// averages values in buffer from start...<finish
static int CalculateValues(int start, int finish)
{
  calcVals.x = calcVals.y = calcVals.z = calcVals.m = 0;

  int i = start;
  int count = 0;
  do
  {
    calcVals.x += dataBuffers[i].x;
    calcVals.y += dataBuffers[i].y;
    calcVals.z += dataBuffers[i].z;
    calcVals.m += dataBuffers[i].m;
    ++count;

    if (++i >= MAXNUM_DATA_VALUES) i = 0;
  }
  while (i != finish);

  calcVals.x = calcVals.x / count;
  calcVals.y = calcVals.y / count;
  calcVals.z = calcVals.z / count;
  calcVals.m = calcVals.m / count;

  #if DEBUG_CALC_DATA
  DBGOUT((F("Calc: X=%d Y=%d Z=%d M=%d C=%d msecs=%d"),
          calcVals.x, calcVals.y, calcVals.z, calcVals.m,
          count, millis()));
  #endif

  return count;
}

// Determines if data satisfies criteria for capture event
static bool HaveCaptureThreshold(void)
{
  int count = CalculateValues(dataIndexCapture, dataIndexWrite);

  // wait for enough data that corresponds to minimum time
  if ((count * MSECS_LOOPTIME) >= MSECS_TO_CAPTURE)
  {
    #if DEBUG_CAPTURE_DATA
    DBGOUT((F("Capture: X=%d Y=%d Z=%d M=%d C=%d I=%d-%d"),
            calcVals.x, calcVals.y, calcVals.z, calcVals.m,
            count, dataIndexCapture, dataIndexWrite));
    #endif

    // averaged magnitude must be minimal value
    if (calcVals.m >= MIN_MAG_CAPTURE)
    {
      if (captureMax.m < calcVals.m)
        captureMax = calcVals;

      captureCount = count;
      return true;
    }
  }

  return false;
}

// Changes to a new pattern, which is determined from the
// calculated values of the previous capture event.
static void DoChangePattern(void)
{
  bool haveX, haveY, haveZ, haveXY, haveXZ, haveYZ;

  haveX = (((captureMax.x - captureMax.y) > MIN_AVEDIFF_TOWIN) && ((captureMax.x - captureMax.z) > MIN_AVEDIFF_TOWIN));
  haveY = (((captureMax.y - captureMax.x) > MIN_AVEDIFF_TOWIN) && ((captureMax.y - captureMax.z) > MIN_AVEDIFF_TOWIN));
  haveZ = (((captureMax.z - captureMax.x) > MIN_AVEDIFF_TOWIN) && ((captureMax.z - captureMax.y) > MIN_AVEDIFF_TOWIN));

  if (!haveX && !haveY && !haveZ)
  {
    haveXY = (((captureMax.x - captureMax.z) > MIN_AVEDIFF_TOWIN) && ((captureMax.y - captureMax.z) > MIN_AVEDIFF_TOWIN));
    haveXZ = (((captureMax.x - captureMax.y) > MIN_AVEDIFF_TOWIN) && ((captureMax.z - captureMax.y) > MIN_AVEDIFF_TOWIN));
    haveYZ = (((captureMax.y - captureMax.x) > MIN_AVEDIFF_TOWIN) && ((captureMax.z - captureMax.x) > MIN_AVEDIFF_TOWIN));
  }
  else haveXY = haveXZ = haveYZ = false;

  #if DEBUG_CAPTURE_DONE
  DBGOUT((F("Capture: %sX=%-3d %sY=%-3d %sZ=%-3d M=%-3d C=%d"),
          ((haveX || haveXY || haveXZ) ? "*" : " "), captureMax.x,
          ((haveY || haveXY || haveYZ) ? "*" : " "), captureMax.y,
          ((haveZ || haveXZ || haveYZ) ? "*" : " "), captureMax.z,
          captureMax.m, captureCount));
  #endif

       if (haveX)  curPattern = 2;
  else if (haveY)  curPattern = 3;
  else if (haveZ)  curPattern = 4;
  else if (haveXY) curPattern = 5;
  else if (haveYZ) curPattern = 6;
  else if (haveXZ) curPattern = 7;
  else             curPattern = 8;
  GetCurPattern(cmdStr);

  stepsFadeAdjust = STEPS_DIMMER(captureMax.m);
  stepsFadeCount = stepsFadeAdjust;
  maxBright = MAX_BRIGHT;

  #if DEBUG_CAPTURE_DONE && DEBUG_OUTPUT
  uint32_t msecs = MSECS_DIMMER(captureMax.m);
  DBGOUT((F("Capture: pattern=%d mag=%d msecs=%d steps=%d"),
            curPattern, captureMax.m, msecs, stepsFadeAdjust));
  #endif
}

// Returns true if capture finished and have set the pattern
// Otherwise, not in capture mode, or not finished yet
static bool CheckCaptureDone(void)
{
  if (captureTime <= stateTime)
  {
    if (captureStart)
    {
      // add to time spent waiting for capture done
      captureMsecs += MSECS_BETWEEN_CALCS + captureTime - stateTime;
    }
    captureTime = stateTime + MSECS_BETWEEN_CALCS;

    if (HaveCaptureThreshold())
    {
      if (!captureStart)
      {
        // begin wait for capture mode to finish
        captureStart = true;
        captureMsecs = 0;
        captureMax.x = captureMax.y = captureMax.z = captureMax.m = 0;
        LED_TURN_ON;
        #if DEBUG_CAPTURE_DONE
        DBGOUT((F("Capture starting: mag=%d count=%d"), calcVals.m, captureCount));
        #endif
      }
    }
    else if (captureStart)
    {
      LED_TURN_OFF;
      captureStart = false;
      DoChangePattern();
      return true;
    }
  }

  return false;
}

// Determines if data satisfies criteria for trigger event
static bool CheckForTrigger(void)
{
  if (triggerTime <= stateTime)
  {
    int count = CalculateValues(dataIndexTrigger, dataIndexWrite);

    // wait for enough data that corresponds to minimum time
    if ((count * MSECS_LOOPTIME) >= MSECS_TO_TRIGGER)
    {
      #if DEBUG_TRIGGER_DATA
      DBGOUT((F("Trigger: X=%d Y=%d Z=%d M=%d C=%d I=%d-%d"),
              calcVals.x, calcVals.y, calcVals.z, calcVals.m,
              count, dataIndexTrigger, dataIndexWrite));
      #endif

      // averaged magnitude must be minimal value
      if (calcVals.m >= MIN_MAG_TRIGGER)
      {
        short force = pixelNutSupport.mapValue(calcVals.m, MIN_MAG_TRIGGER, MAX_MAG_VALUE, 0, MAX_FORCE_VALUE);
        if (force > MAX_FORCE_VALUE) force = MAX_FORCE_VALUE;
        pPixelNutEngine->triggerForce(force);

        #if DEBUG_TRIGGER_ACTION
        DBGOUT((F("Triggering: M=%d C=%d Force=%d"), calcVals.m, count, force));
        #endif

        triggerTime = stateTime + MSECS_TRIG_REARM;
        return true;
      }
    }

    triggerTime = stateTime + MSECS_BETWEEN_CALCS;
  }

  return false;
}

static void CheckExecCmd(char *instr)
{
  if (instr[0]) // if have new command for engine
  {
    PixelNutEngine::Status status = pPixelNutEngine->execCmdStr(instr);
    if (status != PixelNutEngine::Status_Success)
    {
      DBGOUT((F("CmdErr: %d"), status));
      ErrorHandler(2, status, false); // blink for error and continue
    }

    instr[0] = 0; // must clear command string after finished
  }
}

void setup(void)
{
  SetupLED();             // status LED: indicate in setup now
  SetupDBG();             // setup/wait for debug monitor

  AccelerometerInit();    // initialize particular accelerometer being used
  CalibrateValues();      // calibrate to compensate for gravity/orientation

  CheckForPatterns();     // check for internal patterns, fail if none and required
  GetCurPattern(cmdStr);  // get pattern string corresponding to that pattern number
  CheckExecCmd(cmdStr);   // load that pattern into the engine: ready to be displayed

  randomSeed(analogRead(APIN_SEED)); // set seed to value read from unconnected analog port

  BlinkStatusLED(0, 2);   // indicate success
  DBGOUT((F("** Setup complete **")));

  loopTime = triggerTime = captureTime = faderTime = millis();
  faderWaiting = false;
}

void loop(void)
{
  uint32_t time = millis();

  // execute loop at specified intervals regardless of processor
  if (time <= loopTime) return;
  //DBGOUT((F("LoopTime: %d msecs"), (millis() - loopTime)));
  loopTime = time + MSECS_LOOPTIME;

  ProcessInputs(); // always read input first

  #if DEBUG_POINTS_STOP
  if (dataCountCapture < DATACOUNT_CAPTURE) return;
  PrintDataAndStop(); // never returns
  #endif

  #if defined(DPIN_FADER_BUTTON)
  if (button_fader.CheckForChange() != UIDeviceButton::Retcode_NoChange)
  {
    faderEnabled = !faderEnabled;
    DBGOUT((F("Switching modes to %s"), faderEnabled ? "Fader" : "Player"));
    if (!faderEnabled && (theState != STATE_CAPTURE))
      theState = STATE_RESTART; // return to capture mode
  }
  #endif

  if (stateTime <= time)
  {
    stateTime = millis();

    switch (theState)
    {
      default:
      case STATE_CAPTURE:
      {
        if (faderEnabled && faderWaiting && (faderTime < stateTime))
        {
          #if DEBUG_STATE_CHANGE
          DBGOUT((F("Fading begins now")));
          #endif
          theState = STATE_FADING;
          faderWaiting = false;
          break;
        }

        if (!CheckCaptureDone()) CheckForTrigger();

        else if (faderEnabled)
        {
          uint32_t addtime = MSECS_TOPLAY(captureMsecs) + MSECS_MIN_PLAYING;
          faderTime = stateTime + addtime;
          faderWaiting = true;
          #if DEBUG_STATE_CHANGE
          DBGOUT((F("Fade starting in %d msecs..."), addtime));
          #endif

        }
        else ClearBuffers();
        break;
      }
      case STATE_FADING:
      {
        if (!CheckCaptureDone()) CheckForTrigger();

        if (!stepsFadeCount--)
        {
          --maxBright;
          stepsFadeCount = stepsFadeAdjust;
        }

        if (maxBright <= MIN_BRIGHT)
        {
          #if DEBUG_STATE_CHANGE
          DBGOUT((F("Finale begins now")));
          #endif

          theState = STATE_FINALE;
          stepsFadeCount = STEPS_FINALE_BRIGHT;
        }

        pPixelNutEngine->setMaxBrightness(maxBright);
        break;
      }
      case STATE_FINALE:
      {
        if (maxBright <= MAX_BRIGHT)
        {
          int hue = (MAX_BRIGHT - maxBright) * 3;
          sprintf(cmdStr, "P E0 H%d T G", hue); // sequence hue from ~240-0

          if (!--stepsFadeCount)
          {
            ++maxBright;
            stepsFadeCount = STEPS_FINALE_BRIGHT;
          }
        }
        else
        {
          #if DEBUG_STATE_CHANGE
          DBGOUT((F("Capture enabled in %d msecs"), MSECS_MIN_DARKNESS));
          #endif

          maxBright = 0;
          theState = STATE_RESTART;
          stateTime += MSECS_MIN_DARKNESS;
        }

        pPixelNutEngine->setMaxBrightness(maxBright);
        break;
      }
      case STATE_RESTART:
      {
        #if DEBUG_STATE_CHANGE
        DBGOUT((F("Capture in %d msecs"), MSECS_MIN_BREATHING));
        #endif

        curPattern = 1;
        GetCurPattern(cmdStr);
        pPixelNutEngine->setMaxBrightness(maxBright = MAX_BRIGHT);

        theState = STATE_CAPTURE;
        stateTime += MSECS_MIN_BREATHING;

        ClearBuffers();
        break;
      }
    }
  }

  CheckExecCmd(cmdStr);
  if (doUpdate && pPixelNutEngine->updateEffects())
    pNeoPixels->show(pPixelData, (PIXEL_COUNT*3));
}

#endif // XAPP_FIREFLY
//*********************************************************************************************
