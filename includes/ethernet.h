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
#if defined(SPARK)

#define SPARK_SUBNAME_DEVICE  "particle/device/name"

#if WEB_SERVER
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

#if WEB_SERVER
uint32_t inctime = 1000; // 1 second checks for incoming messages
uint32_t nextime = 0;
#endif

//private:

void handler(const char *name, const char *data)
{
  if (!strcmp(name, "PixelNutCommand"))
  {
    DBGOUT((F("Particle cmd: %s"), data));
    strcpy(usercmd, data);
    havecmd = true;
  }
  else if (!strcmp(name, SPARK_SUBNAME_DEVICE))
  {
    DBGOUT((F("Saving device name: %s"), data));
    FlashSetName((char*)data);
  }
  DBG(else DBGOUT((F("Subscribe: name=%s data=%s"), name, data));)
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
  DBGOUT((F("  Version=%s"), System.version().c_str()));
  System.deviceID().getBytes((byte*)cmdStr, STRLEN_PATTERNS);
  DBGOUT((F("  DeviceID=%s"), cmdStr));
  DBGOUT((F("  Memory=%d free bytes"), System.freeMemory()));
  Time.timeStr().getBytes((byte*)cmdStr, STRLEN_PATTERNS);
  DBGOUT((F("  Time=%s"), cmdStr));

#if !USE_SOFTAP
  DBGOUT((F("Waiting for connection...")));
  Particle.connect();

  uint32_t tout = millis() + 3000;
  while (!Particle.connected())
  {
    Particle.process();
    BlinkStatusLED(1, 0);
    if (millis() > tout) break;
  }
  if (!WiFi.ready())
  {
    DBGOUT((F("...connection failed")));
    ErrorHandler(3, 1, true); // does not return from this call
  }
  DBGOUT((F("...network found")));

#if DEBUG_OUTPUT
  Serial.print("LocalIP: "); Serial.println(WiFi.localIP());
  Serial.print("Subnet:  "); Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
  Serial.print("SSID:    "); Serial.println(WiFi.SSID());
  /*
  if (WiFi.hasCredentials())
  {
    WiFiAccessPoint ap[5];
    int count = WiFi.getCredentials(ap, 5);
    DBGOUT((F("Credentials=%d"), count));
    for (int i = 0; i < count; i++)
    DBGOUT((F("  %d) SSID: %s"), i+1, ap[i].ssid));
  }
  */
#endif
#endif

  if (!Particle.subscribe("PixelNutCommand", &Ethernet::handler, this))
  {
    DBGOUT((F("Particle subscribe failed")));
    ErrorHandler(3, 2, true); // does not return from this call
  }
  if (!Particle.publish("PixelNutResponse", "PixelNut! is online", 60, PRIVATE))
  {
    DBGOUT((F("Particle publish failed")));
    ErrorHandler(3, 3, true); // does not return from this call
  }

  char flashname[MAXLEN_DEVICE_NAME+1];
  FlashGetName(flashname);
  if (!flashname[0])
  {
    if (!Particle.subscribe(SPARK_SUBNAME_DEVICE, &Ethernet::handler, this) ||
        !Particle.publish(SPARK_SUBNAME_DEVICE))
    {
      DBGOUT((F("Retrieving device name failed")));
      ErrorHandler(3, 4, true); // does not return from this call
    }
  }

  #if WEB_SERVER
  DBGOUT((F("Starting web server...")));
  webServer.begin();
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

  if (!Particle.publish("PixelNutResponse", instr))
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

  #if WEB_SERVER
  uint32_t thetime = pixelNutSupport.getMsecs();

  if (thetime > nextime) // callbacks every second
  {
    if (webClient.connected() && webClient.available())
    {
      //DBGOUT((F("Web is connected!!")));

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

  if (!havecmd) return false;

  strcpy(cmdStr, usercmd);
  havecmd = false;

  DBGOUT((F("Web --> \"%s\""), cmdStr));
  if (pAppCmd->execCmd()) CheckExecCmd();

  return true;
};

#endif // defined(SPARK)
#endif // ETHERNET_COMM
//========================================================================================
