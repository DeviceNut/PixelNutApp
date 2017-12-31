// PixelNutApp Bluetooth Communications
//
// Uses global variables: 'pixelNutSupport', 'cmdStr', 'pAppCmd'.
// Calls global routines: 'CheckExecCmd', 'ErrorHandler'.
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if ETHERNET_COMM

#if BE_WEB_SERVER
TCPServer webServer = TCPServer(80);
TCPClient webClient;
#endif

SYSTEM_MODE(SEMI_AUTOMATIC);

extern void CheckExecCmd();   // defined in main.h
extern AppCommands *pAppCmd;  // pointer to current instance

class Ethernet : public CustomCode
{
public:

  void setup(void);
  bool control(void);

  bool setName(char *name);
  bool sendReply(char *instr);

  bool inSetup = false;
  bool isConnected = false;

  bool havecmd;
  char usercmd[STRLEN_PATTERNS];
  uint16_t update_counter;
  uint32_t timePublished;
  byte countPublished;

  #if BE_WEB_SERVER
  uint32_t inctime = 1000; // 1 second checks for incoming messages
  uint32_t nextime = 0;
  #endif

private:

  void handler(const char *name, const char *data)
  {
    if (!strcmp(name, "command"))
    {
      strcpy(usercmd, data);
      havecmd = true;
    }
    DBG( else DBGOUT((F("Subscribe name unknown: %s"), name)); )
  }
};
Ethernet ethernet;

void Ethernet::setup(void)
{
  inSetup = true;
  isConnected = false;

  timePublished = pixelNutSupport.getMsecs();
  countPublished = 0;
  update_counter = 0;
  havecmd = false;

  DBGOUT((F("Particle Photon:")));
  DBGOUT((F("  Version=%s"),      System.version().c_str()));
  System.deviceID().getBytes((byte*)cmdStr, STRLEN_PATTERNS);
  DBGOUT((F("  DeviceID=%s"),     cmdStr));
  DBGOUT((F("  Memory=%d bytes"), System.freeMemory()));
  Time.timeStr().getBytes((byte*)cmdStr, STRLEN_PATTERNS);
  DBGOUT((F("  Time=%s bytes"),   cmdStr));

  if (!Particle.subscribe("command", &Ethernet::handler, this))
  {
    DBGOUT((F("Spark registration failed")));
    return;
  }

  #if BE_WEB_SERVER
  DBGOUT((F("Starting web server...")));
  webServer.begin();
  #else
  DBGOUT((F("Waiting for connection...")));
  Particle.connect();

  uint32_t tout = millis() + 3000;
  while (!Particle.connected())
  {
    Particle.process();
    BlinkStatusLED(1,0);
    if (millis() > tout) break;
  }
  if (!WiFi.ready())
  {
    DBGOUT((F("...connection failed")));
    return;
  }
  DBGOUT((F("...network found")));

  #if DEBUG_OUTPUT
  Serial.print("LocalIP: "); Serial.println(WiFi.localIP());
  Serial.print("Subnet:  "); Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
  Serial.print("SSID:    "); Serial.println(WiFi.SSID());
  #endif
  #endif

  inSetup = false;
}

// return false if failed to set name
bool Ethernet::setName(char *name)
{
  return true;
}

// return false if failed to send message
bool Ethernet::sendReply(char *instr)
{
  DBGOUT((F("Web <-- \"%s\""), instr));
  bool success = true;

  // can only publish 4 at once, then once a second
  uint32_t msecs = (pixelNutSupport.getMsecs() - timePublished);
  if (msecs < 1000)
  {
    if (++countPublished >= 4)
      delay(1000 - msecs);
  }
  else countPublished = 0;

  if (!Particle.publish("response", instr))
  {
    DBGOUT((F("Particle publish failed")));
    success = false;
  }

  timePublished = pixelNutSupport.getMsecs();

  return success;
}

// return true if have command input
bool Ethernet::control(void)
{
  // check if already have command input
  if (cmdStr[0] != 0)
  {
    DBGOUT((F("Web: have input=\"%s\""), cmdStr));
    return true;
  }

  #if BE_WEB_SERVER
  uint32_t thetime = pixelNutSupport.getMsecs();

  if (thetime > nextime) // callbacks every 
  {
    if (webClient.connected() && webClient.available())
    {
      DBGOUT((F("Web is connected!!")));

      webClient.print("<html><body>PixelNut! Time: ");
      webClient.print(millis()/1000);
      webClient.println(" seconds</body></html>\n\n");
      //while (webClient.status() == 1); // wait until socket is not active
      webClient.flush();
      //webClient.stop();
    }
    else webClient = webServer.available();

    nextime = thetime + inctime;
  }
  #endif

  if (havecmd)
  {
    strcpy(cmdStr, usercmd);
    havecmd = false;

    DBGOUT((F("Web --> \"%s\""), cmdStr));
    if (pAppCmd->execCmd()) CheckExecCmd();

    return true;
  }

  return false;  // allow for alternative command input
};

#endif // ETHERNET_COMM
//========================================================================================
