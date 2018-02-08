// PixelNutApp Custom Code: default handling class methods
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

class CustomCode
{
public:

  #if EEPROM_FORMAT
  virtual void flash(void) {}
  #endif

  // called during setup()
  virtual void setup(void) {}
  
  // called during loop()
  virtual bool loop(void)
  {
    // return true if handling commands externally
    return false; // check physical controls
  }

  // called in loop() if have new pattern in engine
  virtual void pattern(void) {}

  // override for external communications
  virtual bool setName(char *name) { return false; }
  virtual bool sendReply(char *instr) { return false; }
};

#if !CUSTOM_OVERRIDE
CustomCode customcode;
CustomCode *pCustomCode = &customcode;
#else
extern CustomCode *pCustomCode;
#endif

//========================================================================================
