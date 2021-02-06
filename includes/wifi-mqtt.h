// PixelNutApp WiFi Communications using MQTT
//
// Uses global variables: 'pixelNutSupport', 'pAppCmd'.
// Calls global routines: 'CheckExecCmd'.
//========================================================================================
/*
Copyright (c) 2021, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

/*****************************************************************************************
 Protocol used with MQTT:

 1) This Client has the Wifi credentials and the Broker's IP hardcoded.

 2) Client sends to Broker (topic="PixelNutNotify"): <DevName> <IPaddr>
    IPaddr: local ip address (e.g. 192.168.1.122)
    DevName: Friendly name of this device (e.g. "My Device")
    This is sent periodically to maintain a connection.

 3) Broker sends command to Client (topic is <DevName>): <cmdstr>
    <cmdstr> is a PixelNut command string.

 4) If command starts with "?" then client will reply (topic="PixelNutReply"): <reply>
    <reply> is one or more lines of text with information, depending on the command.

*****************************************************************************************/

#if defined(WIFI_MQTT) && WIFI_MQTT

// Cannot be used with logical segments or custom built-in patterns
#if (!STRANDS_MULTI && (SEGMENT_COUNT > 1))
#error("Logical segments not supported with WiFi/Mqtt")
#endif
#if CUSTOM_PATTERNS
#error("Custom patterns not supported with WiFi/Mqtt")
#endif

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#include "mycredentials.h"
/* Wifi SSID/Password and MQTT Broker in the following format:
#define WIFI_CREDS_SSID "SSID"
#define WIFI_CREDS_PASS "PASSWORD"
#define MQTT_BROKER_IPADDR "192.168.1.4"
#define MQTT_BROKER_PORT 1883
*/

extern void CheckExecCmd(char *instr); // defined in main.h

#define MQTT_TOPIC_BASE       "PixelNut/"
#define MQTT_TOPIC_NOTIFY     "PixelNut/Notify"
#define MQTT_TOPIC_REPLY      "PixelNut/Reply"
#define MAXLEN_DEVICE_IPSTR   15 // aaa.bbb.ccc.ddd
#define MSECS_CONNECT_PUB     3000 // msecs between connect publishes

class WiFiMqtt : public CustomCode
{
public:

  #if EEPROM_FORMAT
  void flash(void) { setName(DEFAULT_DEVICE_NAME); }
  #endif

  void setup(void);
  void loop(void);

  bool setName(char *name);

  PubSubClient mqttClient;

  // creates the topic name for sending cmds
  // needs to be public to be used in callback
  char deviceName[MAXLEN_DEVICE_NAME + 1];

  char replyStr[1000]; // long enough for all segments

private:

  WiFiClient wifiClient;

  char localIP[MAXLEN_DEVICE_IPSTR];  // local IP address

  // topic to subscribe to, with device name
  char devnameTopic[sizeof(MQTT_TOPIC_BASE) + MAXLEN_DEVICE_NAME + 1];
  // string sent to the MQTT_TOPIC_NOTIFY topic
  char connectStr[MAXLEN_DEVICE_IPSTR + MAXLEN_DEVICE_NAME + 1];
  uint32_t nextConnectTime = 0; // next time to send notify string

  void ConnectWiFi(void);   // waits for connection to WiFi
  bool ConnectMqtt(void);   // returns True if now connected

  void SetDeviceName(void);
};
WiFiMqtt wifiMQTT;

void WiFiMqtt::ConnectWiFi(void)
{
  DBGOUT(("Connect to WiFi..."));
  WiFi.begin(WIFI_CREDS_SSID, WIFI_CREDS_PASS);

  while (WiFi.status() != WL_CONNECTED)
    BlinkStatusLED(1, 0);
}

bool WiFiMqtt::ConnectMqtt(void)
{
  if (nextConnectTime >= millis())
    return mqttClient.connected();

  nextConnectTime = millis() + MSECS_CONNECT_PUB;

  if (!mqttClient.connected())
  {
    DBGOUT(("Connect to Mqtt..."));
    if (mqttClient.connect(deviceName))
    {
      DBGOUT(("Subscribing to: %s", devnameTopic));
      mqttClient.subscribe(devnameTopic);
    }
  }

  if (mqttClient.connected())
  {
    mqttClient.publish(MQTT_TOPIC_NOTIFY, connectStr, false);
    return true;
  }

  DBGOUT(("Mqtt State: %s", mqttClient.state()));
  BlinkStatusLED(0, 1);
  return false;
}

void CreateReplyStr(void)
{
  char pstr[STRLEN_PATTERNS];
  char *rstr = wifiMQTT.replyStr;
  rstr[0] = 0;

  #if (SEGMENT_COUNT > 1)
  byte pixcounts[] = PIXEL_COUNTS;
  byte ltcounts[] = LAYER_TRACK_COUNTS;
  #else
  byte pixcounts[] = { PIXEL_COUNT };
  byte ltcounts[] = { NUM_PLUGIN_LAYERS, NUM_PLUGIN_TRACKS, 0 };
  #endif
  byte lcount, tcount;

  sprintf(rstr, "%s\n%d %d\n", wifiMQTT.deviceName, SEGMENT_COUNT, STRLEN_PATTERNS);
  rstr += strlen(rstr);

  for (int i = 0; i < SEGMENT_COUNT; ++i)
  {
    FlashSetSegment(i);

    if (ltcounts[i*2] != 0)
    {
      lcount = ltcounts[i*2];
      tcount = ltcounts[i*2 + 1];
    }

    FlashGetStr(pstr);
    sprintf(rstr, "%d %d %d %d %d\n%s", pixcounts[i], lcount, tcount,
                  FlashGetValue(FLASH_SEG_BRIGHTNESS),
                  FlashGetValue(FLASH_SEG_DELAYMSECS), pstr);
    rstr += strlen(rstr);
  }

  FlashSetSegment(curSegment = 0); // MUST start with segment 0
}

void CallbackMqtt(char* topic, byte* message, unsigned int msglen)
{
  //DBGOUT(("Callback for topic: %s", topic));

  // ignore 'topic' as there is only one
  if (msglen <= 0) return;

  while (*message == ' ')
  {
    ++message; // skip spaces
    --msglen;
    if (msglen <= 0) return;
  }

  if (msglen < STRLEN_PATTERNS) // msglen doesn't include terminator
  {
    strncpy(cmdStr, (char*)message, msglen);
    cmdStr[msglen] = 0;
    DBGOUT(("Process: \"%s\"", cmdStr));

    if (*cmdStr == '?')
    {
      CreateReplyStr();
      DBGOUT(("ReplyStr: \"%s\"", wifiMQTT.replyStr));
      wifiMQTT.mqttClient.publish(MQTT_TOPIC_REPLY, wifiMQTT.replyStr);
      *cmdStr = 0; // MUST clear string before returning
    }
    // all other PixelNut commands, without a reply
    else if (pAppCmd->execCmd(cmdStr)) CheckExecCmd(cmdStr);
  }
  else { DBGOUT(("MQTT message too long: %d bytes", msglen)); }
}

void WiFiMqtt::SetDeviceName(void)
{
  FlashGetName(deviceName);

  strcpy(devnameTopic, MQTT_TOPIC_BASE);
  strcat(devnameTopic, deviceName);

  strcpy(connectStr, deviceName);
  strcat(connectStr, " ");
  strcat(connectStr, localIP);
}

void WiFiMqtt::setup(void)
{
  DBGOUT(("---------------------------------------"));

  #if defined(ESP32)
  esp_chip_info_t sysinfo;
  esp_chip_info(&sysinfo);
  DBGOUT(("ESP32 Board:"));
  DBGOUT(("  SDK Version=%s", esp_get_idf_version()));
  DBGOUT(("  ModelRev=%d.%d", sysinfo.model, sysinfo.revision));
  DBGOUT(("  Cores=%d", sysinfo.cores));
  DBGOUT(("  Heap=%d bytes", esp_get_free_heap_size()));
  #endif

  ConnectWiFi();
  mqttClient.setClient(wifiClient);
  mqttClient.setServer(MQTT_BROKER_IPADDR, MQTT_BROKER_PORT);
  mqttClient.setCallback(CallbackMqtt);

  strcpy(localIP, WiFi.localIP().toString().c_str());
  SetDeviceName();

  DBGOUT(("Device \"%s\":", deviceName));
  DBGOUT(("  LocalIP=%s", localIP));
  DBGOUT(("  Broker=%s:%d", MQTT_BROKER_IPADDR, MQTT_BROKER_PORT));
  DBGOUT(("  MaxBufSize=%d", MQTT_MAX_PACKET_SIZE));
  DBGOUT(("  KeepAliveSecs=%d", MQTT_KEEPALIVE));

  DBGOUT(("---------------------------------------"));
}

bool WiFiMqtt::setName(char *name)
{
  FlashSetName(name);
  SetDeviceName();

  // re-connect with new name next loop
  mqttClient.disconnect();
  nextConnectTime = 0;

  return true;
}

void WiFiMqtt::loop(void)
{
    if (ConnectMqtt()) mqttClient.loop();
}

#endif // MQTT_ESP32
//========================================================================================
