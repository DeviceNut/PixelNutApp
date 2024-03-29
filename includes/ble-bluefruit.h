// PixelNutApp Bluetooth Communications using Bluefruit Module
//
// Uses global variables: 'pixelNutSupport', 'cmdStr', 'pAppCmd'.
// Calls global routines: 'CheckExecCmd', 'ErrorHandler'.
//========================================================================================
/*
Copyright (c) 2015-2020, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if defined(BLE_BLUEFRUIT) && BLE_BLUEFRUIT

#include <SPI.h>

BluefruitStrs bfruit(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

extern void CheckExecCmd(char *instr); // defined in main.h

#define MSECS_CHECK_CONNECT   500     // check for connection every 1/2 second

class BleBluefruit : public CustomCode
{
public:

  void setup(void);
  void loop(void);

  bool setName(char *name);
  bool sendReply(char *instr);

  bool inSetup = false;
  bool isConnected = false;

private:

  unsigned long msecsConnect = 0;
};
BleBluefruit bleBluefruit;

// this will be called upon any bluetooth communication errors
// upon return the hardware will be reset automatically for us
void notifyCB(NotifyMessage msgval, char *msgstr)
{
  DBGOUT((F("BleBluefruit Notification %d: %s"), msgval, msgstr));

  DBGOUT((F("Resetting BLE controller...")));
  bfruit.reset(); // hardware reset, this clears cmdStr[] too
  bleBluefruit.isConnected = false;
  DBGOUT((F("Reset completed")));

  ErrorHandler(3, msgval, bleBluefruit.inSetup); // hangs here if in setup
}

static bool CheckDevName(char *devstr)
{
  bool badname = false;
  char name[MAXLEN_DEVICE_NAME+1];

  FlashGetName(name);
  int len = strlen(name);

  if (len < 2) badname = true;
  else for (int i = 0; i < len; ++i)
  {
    char c = name[i];
    if (!isalpha(c) && !isdigit(c) &&
          (c != ' ') && (c != '-')  &&
          (c != '!') && (c != '#')  &&
          (c != '$') && (c != '%')  &&
          (c != '&') && (c != '*'))
    {
      badname = true;
      break;
    }
  }

  bool goodble = (strlen(devstr) > PREFIX_LEN_DEVNAME) &&
                  !strncmp(devstr, PREFIX_DEVICE_NAME, PREFIX_LEN_DEVNAME);

  // if the name stored in flash is invalid or empty,
  // and there isn't what looks like a good BLE name,
  // then reset to a generic name, so that the user
  // can be able to find it and rename it in the app
  if (badname && !goodble)
  {
    strcpy(devstr, DEFAULT_DEVICE_NAME);
    FlashSetName(devstr);
    return true;
  }
  // else if there's a good BLE name but not a
  // good flash name, override the flash name
  else if (badname && goodble)
    FlashSetName(devstr + PREFIX_LEN_DEVNAME);

  // otherwise if there isn't a good BLE name or the
  // name stored in flash doesn't match it, then just
  // reset the BLE name. (This is done because the BLE
  // name in certain Adafruit's devices sometimes gets
  // reset to "Adafruit Bluefruit LE" for some reason.)
  else if (!goodble || strcmp(name, devstr + PREFIX_LEN_DEVNAME))
  {
    strcpy(devstr, name);
    return true;
  }
  
  return false;
}

// return false if failed to set name
bool setNameBLE(char *name)
{
  // 14 chars for command + terminator
  // + beginning prefix to name
  char str[MAXLEN_DEVICE_NAME+PREFIX_LEN_DEVNAME+14+1];
  strcpy(str, (char*)"AT+GAPDEVNAME=");
  strcpy((str+14), PREFIX_DEVICE_NAME);
  strcpy((str+14+PREFIX_LEN_DEVNAME), name);

  DBGOUT((F("Setting BLE name: %s"), str));
  if (!bfruit.sendCmdStr(str, NULL))
  {
    DBGOUT((F("Setting name failed")));
    return false;
  }
  return true;
}

// response is in cmdStr
void getNameCB(void)
{
  if (!strcmp(cmdStr, "OK")) return; // ignore this

  DBGOUT((F("BLE DevName: \"%s\""), cmdStr));

  if (CheckDevName(cmdStr))
    setNameBLE(cmdStr);
}

// does not return if fail because NotifyCB gets
// called which will then hang in ErrorHandler
void BleBluefruit::setup(void)
{
  inSetup = true;
  isConnected = false;

  DBGOUT((F("Setting up bluetooth...")));
  bfruit.init(cmdStr, STRLEN_PATTERNS, notifyCB);

  // this seems to insure a clean start, otherwise
  // it sometimes gets stuck in connected mode:
  bfruit.sendCmdStr((char*)"ATZ", NULL);
  delay(1000); // Bluefruit takes 1 second to reboot

  if (!bfruit.sendCmdStr((char*)"AT+GAPDEVNAME", getNameCB))
  {
    DBGOUT((F("BLE get name failed")));
  }

  inSetup = false;
}

// return false if failed to set name
bool BleBluefruit::setName(char *name)
{
  FlashSetName(name);
  return setNameBLE(name);
}

// return false if failed to send message
// upon exit cmdStr[] has garbage from calling writeDataStr()
bool BleBluefruit::sendReply(char *instr)
{
  DBGOUT((F("BLE Tx: \"%s\""), instr));

  bool success = true;

  // client expects all strings terminated with newline
  if (!bfruit.writeDataStr(instr) ||
      !bfruit.writeDataStr((char*)"\n"))
  {
    DBGOUT((F("BLE write data failed")));
    success = false;
  }

  delay(100); // hack to prevent data overflow with bluefruit
  msecsConnect = pixelNutSupport.getMsecs();

  return success;
}

static void ResponseCB(void)
{
  DBGOUT((F("BLE Rx: \"%s\""), cmdStr));
  if (pAppCmd->execCmd(cmdStr)) CheckExecCmd(cmdStr);
}

void BleBluefruit::loop(void)
{
  if ((pixelNutSupport.getMsecs() - msecsConnect) > MSECS_CHECK_CONNECT)
  {
    //DBGOUT((F("BLE: check for connection...")));
    isConnected = bfruit.isConnected();
    // NOTE: immediately sending data causes failures

    msecsConnect = pixelNutSupport.getMsecs();
  }

  if (isConnected)
  {
    //DBGOUT((F("BLE: reading...")));
    if (!bfruit.readDataStr(ResponseCB))
    {
      DBGOUT((F("BLE read data failed")));
    }
  }
};

#endif // BLE_BLUEFRUIT
//========================================================================================
