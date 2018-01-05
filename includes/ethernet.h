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

extern void CheckExecCmd();   // defined in main.h
extern AppCommands *pAppCmd;  // pointer to current instance

#define SPARK_SUBNAME_DEVICE  "particle/device/name"

#if USE_SOFTAP
#include "softap_http.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

static char htmlReplyString[1000]; // long enough for maximum ?, ?S, ?P output strings

static const char indexpage[] = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'><title>PixelNut!</title></head><body><h1>Hello from PixelNut!</h1></body></html>";

void httpHandler(const char* url, ResponseCallback* cb, void* cbArg, Reader* body, Writer* result, void* reserved)
{
    DBGOUT((F("SoftAP: URL=%s"), url));

    if (!strcmp(url, "/index") || !strcmp(url, "/index.html"))
    {
        cb(cbArg, 0, 200, "text/html", nullptr);
        result->write(indexpage);
    }
    else if (!strcmp(url, "/info"))
    {
        cb(cbArg, 0, 200, "text/plain", nullptr);

        htmlReplyString[0] = 0;
        strcpy(cmdStr, "?");
        pAppCmd->execCmd();

        result->write(htmlReplyString);
    }

    #if (SEGMENT_COUNT > 1)
    else if (!strcmp(url, "/segments"))
    {
        cb(cbArg, 0, 200, "text/plain", nullptr);

        htmlReplyString[0] = 0;
        strcpy(cmdStr, "?S");
        pAppCmd->execCmd();

        result->write(htmlReplyString);
    }
    #endif

    #if CUSTOM_PATTERNS
    else if (!strcmp(url, "/patterns"))
    {
        cb(cbArg, 0, 200, "text/plain", nullptr);

        htmlReplyString[0] = 0;
        strcpy(cmdStr, "?P");
        pAppCmd->execCmd();

        result->write(htmlReplyString);
    }
    #endif

    else if (!strcmp(url, "/command"))
    {
        cb(cbArg, 0, 200, "text/plain", nullptr);

        if (body->bytes_left)
        {
          char *data = body->fetch_as_string();
          strcpy(cmdStr, data);
          free(data);

          if (pAppCmd->execCmd()) CheckExecCmd();
        }

        result->write("ok");
    }
    else cb(cbArg, 0, 404, nullptr, nullptr);
}

// Private network IP address: http://192.168.0.1
STARTUP(softap_set_application_page_handler(httpHandler, nullptr));

#else // !USE_SOFTAP
SYSTEM_MODE(SEMI_AUTOMATIC);
#endif

class Ethernet : public CustomCode
{
public:

  void setup(void);
  bool control(void);

  bool setName(char *name);
  bool sendReply(char *instr);

private:

  bool haveUserCmd = false;
  char userCmdStr[STRLEN_PATTERNS] = {0};

  #if !USE_SOFTAP
  uint16_t update_counter;
  uint32_t timePublished;
  byte countPublished;
  #endif

  void handler(const char *name, const char *data)
  {
    if (!strcmp(name, "PixelNutCommand"))
    {
      DBGOUT((F("Particle cmd: %s"), data));
      strcpy(userCmdStr, data);
      haveUserCmd = true;
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
  DBGOUT((F("Particle Photon:")));
  DBGOUT((F("  Version=%s"), System.version().c_str()));
  System.deviceID().getBytes((byte*)cmdStr, STRLEN_PATTERNS);
  DBGOUT((F("  DeviceID=%s"), cmdStr));
  DBGOUT((F("  Memory=%d free bytes"), System.freeMemory()));
  Time.timeStr().getBytes((byte*)cmdStr, STRLEN_PATTERNS);
  DBGOUT((F("  Time=%s"), cmdStr));

  #if USE_SOFTAP
  System.set(SYSTEM_CONFIG_SOFTAP_PREFIX, "P!MyDevice");
  System.set(SYSTEM_CONFIG_SOFTAP_SUFFIX, "!");
  WiFi.listen();
  #else
  timePublished = pixelNutSupport.getMsecs();
  countPublished = 0;
  update_counter = 0;

  DBGOUT((F("Waiting for connection...")));
  Particle.connect();

  uint32_t tout = millis() + 3000;
  while (!WiFi.ready())
  {
    Particle.process();
    BlinkStatusLED(1, 0);
    if (millis() > tout)
    {
      DBGOUT((F("...connection failed")));
      ErrorHandler(3, 1, true); // does not return from this call
    }
  }

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
  #endif
}

// return false if failed to set name
bool Ethernet::setName(char *name)
{
  FlashSetName((char*)name);
  return true;
}

// return false if failed to send message
bool Ethernet::sendReply(char *instr)
{
  DBGOUT((F("Web <-- \"%s\""), instr));
  bool success = true;

  #if USE_SOFTAP
  strcat(htmlReplyString, instr);
  strcat(htmlReplyString, "\r\n");
  #else
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
  #endif

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

  if (!haveUserCmd) return false;
  strcpy(cmdStr, userCmdStr);
  userCmdStr[0] = 0;
  haveUserCmd = false;

  DBGOUT((F("Web --> \"%s\""), cmdStr));
  if (pAppCmd->execCmd()) CheckExecCmd();

  return true;
};

#endif // defined(SPARK)
#endif // ETHERNET_COMM
//========================================================================================
