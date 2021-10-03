// PixelNutApp Bluetooth Communications on ESP32 Boards
//
// Uses global variables: 'pixelNutSupport', 'cmdStr', 'pAppCmd'.
// Calls global routines: 'CheckExecCmd', 'ErrorHandler'.
//========================================================================================
/*
Copyright (c) 2021, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if defined(BLE_ESP32) && BLE_ESP32

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

extern void CheckExecCmd(char *instr); // defined in main.h

#define SERVICE_UUID_UART   "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_UUID_UART_RX   "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_UUID_UART_TX   "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define XFER_CHUNK_SIZE     20  // max bytes of data in each xfer
#define MAXNUM_QSTRS        10  // max number of strings in queue

class BleEsp32 : public CustomCode
{
public:

  void setup(void);
  void loop(void);

  bool setName(char *name);
  bool sendReply(char *instr);

  char deviceName[MAXLEN_DEVICE_NAME + 1];

  bool isConnected = false;

  char queueStrs[MAXNUM_QSTRS][STRLEN_PATTERNS] = {};
  int queueHead = 0;
  int queueTail = 0;

private:

  BLEServer *pServer;
  BLEService *pService;
  BLECharacteristic *pRxChar;
  BLECharacteristic *pTxChar;
  BLEAdvertising *pAdvertising;
};
BleEsp32 bleEsp32;

class ServerCallbacks: public BLEServerCallbacks
{
  void onConnect(BLEServer* pServer)
  {
    DBGOUT(("BLE Connect"));
    bleEsp32.isConnected = true;
  };

  void onDisconnect(BLEServer* pServer)
  {
    DBGOUT(("BLE Disconnect"));
    bleEsp32.isConnected = false;
  }
};

class CharCallbacks: public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    static char *qcurstr = NULL;  // pointer into queue string
    static int qindex;            // queue index of that string
    static int qcurlen;           // chars left in that string
    
    char tmpstr[XFER_CHUNK_SIZE+1];
    char *instr = tmpstr;

    const char *blestr = pCharacteristic->getValue().c_str();
    int inlen = strlen(blestr);
    if (inlen > XFER_CHUNK_SIZE)
    {
        DBGOUT(("BLE input length=%d", inlen));
        ErrorHandler(3, 1, false);
        return;
    }
    strcpy(instr, blestr);
    DBGOUT(("BLE read: \"%s\"", instr));

    while (*instr) // while more string to parse
    {
      if (qcurstr == NULL)
      {
        qindex = (bleEsp32.queueTail + 1) % MAXNUM_QSTRS;
        if (qindex == bleEsp32.queueHead)
        {
            DBGOUT(("BLE queue too small"));
            ErrorHandler(3, 2, false);
            return;
        }

        DBGOUT(("BLE queue index=%d", qindex));
        qcurstr = bleEsp32.queueStrs[bleEsp32.queueTail];
        qcurlen = STRLEN_PATTERNS-1;
      }

      char *pnext = strchr(instr, '\n');
      int inlen;

      if (pnext == NULL) // no end-of-line
           inlen = strlen(instr);
      else inlen = (pnext - instr);

      if (inlen > qcurlen)
      {
        DBGOUT(("BLE command too long"));
        ErrorHandler(3, 3, false);
        qcurstr = NULL; // toss current command
        return;
      }

      if (pnext == NULL) // no end-of-line
      {
        DBGOUT(("BLE save chunk: \"%s\"", instr));
        strcpy(qcurstr, instr);
        qcurstr += inlen;
        qcurlen -= inlen;
        instr += inlen;
      }
      else
      {
        strncpy(qcurstr, instr, inlen);
        qcurstr[inlen] = 0;

        bleEsp32.queueTail = qindex;  // release command string
        qcurstr = NULL;         // get next string in queue
        instr = pnext + 1;      // skip to next line of input
      }
    }
  }
};

void BleEsp32::setup(void)
{
  DBGOUT(("---------------------------------------"));

  DBGOUT((F("Setting up BLE...")));

  FlashGetName(deviceName);
  char str[PREFIX_LEN_DEVNAME + MAXLEN_DEVICE_NAME + 1];
  strcpy(str, PREFIX_DEVICE_NAME);
  strcpy((str+PREFIX_LEN_DEVNAME), deviceName);

  BLEDevice::init(str);

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
  pService = pServer->createService(SERVICE_UUID_UART);

  pTxChar = pService->createCharacteristic(CHAR_UUID_UART_TX,
						              BLECharacteristic::PROPERTY_NOTIFY);
  pTxChar->addDescriptor(new BLE2902());

  pRxChar = pService->createCharacteristic(CHAR_UUID_UART_RX,
											    BLECharacteristic::PROPERTY_WRITE);
  pRxChar->setCallbacks(new CharCallbacks());

  pService->start();

  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID_UART);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // helps with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  DBGOUT(("BLE service started"));
  DBGOUT(("---------------------------------------"));
}

// return false if failed to set name
bool BleEsp32::setName(char *name)
{
  FlashSetName(name);
  strcpy(deviceName, name);
  return true;
}

// return false if failed to send message
bool BleEsp32::sendReply(char *instr)
{
  if (isConnected)
  {
    DBGOUT(("BLE Tx: \"%s\"", instr));

    // break into chunks, add newline at end
    char outstr[XFER_CHUNK_SIZE+2]; // +1 newline, +1 terminator
    int slen = strlen(instr);
    char *pstr = instr;

    while (*pstr)
    {
      int copylen = (slen > XFER_CHUNK_SIZE) ? XFER_CHUNK_SIZE : slen;
      strncpy(outstr, pstr, copylen);
      outstr[copylen] = 0;
      slen -= copylen;
      pstr += copylen;

      //DBGOUT(("BLE send chunk: \"%s\"", outstr));

      if (!*pstr) strcat(outstr, "\n");

      pTxChar->setValue(outstr);
      pTxChar->notify();
      delay(10);
    }
  }

  return true;
}

void BleEsp32::loop(void)
{
  if (bleEsp32.queueHead != bleEsp32.queueTail)
  {
    char *rxstr = bleEsp32.queueStrs[bleEsp32.queueHead];
    DBGOUT(("BLE Rx: \"%s\"", rxstr));

    strcpy(cmdStr, rxstr);
    if (pAppCmd->execCmd(cmdStr)) CheckExecCmd(cmdStr);

    *rxstr = 0; // must clear for the producer
    bleEsp32.queueHead = (bleEsp32.queueHead + 1) % MAXNUM_QSTRS;
  }
};

#endif // BLE_ESP32
//========================================================================================
