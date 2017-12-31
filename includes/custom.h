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

  // called during setup()
  virtual void setup(void) {}
  
  // return true if have command input
  virtual bool control(void)
  {
    return (cmdStr[0] != 0); // true if already have command
  }

  // called in loop() if have new pattern to display
  virtual void display(void) {}

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
