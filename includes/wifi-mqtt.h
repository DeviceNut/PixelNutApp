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

 1) This Client has the Wifi credentials and the Broker's IP hardcoded in flash.

 2) Client sends to Broker (topic="PixelNutName"): <DevName> <IPaddr>
    IPaddr: local ip address (e.g. 192.168.1.122)
    DevName: Friendly name of this device (e.g. "My Device")
    This is sent every second to maintain a connection.

 3) Broker sends command to Client (topic is <DevName>): <cmdstr>
    <cmdstr> is a PixelNut command string.

 4) If command starts with "?" then client will reply (topic="PixelNutReply"): <reply>
    <reply> is one or more lines of text with information, depending on the command.

*****************************************************************************************/

#if defined(WIFI_MQTT) && WIFI_MQTT

#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#include "mycredentials.h"
/* Wifi SSID/Password and MQTT Broker in the following format:
#define WIFI_CREDS_SSID "SSID"
#define WIFI_CREDS_PASS "PASSWORD"
#define MQTT_CREDS_IPADDR "192.168.1.4"
#define MQTT_CREDS_PORT 1883
*/

extern void CheckExecCmd(char *instr); // defined in main.h

#define MQTT_TOPIC_NOTIFY     "PixelNutNotify"
#define MQTT_TOPIC_REPLY      "PixelNutReply"
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
  bool sendReply(char *instr);

  PubSubClient mqttClient;
  char replyString[1000]; // long enough for maximum ?, ?S, ?P output strings

private:

  WiFiClient wifiClient;

  uint32_t nextConnectTime = 0;

  // creates the topic name for sending cmds
  char deviceName[MAXLEN_DEVICE_NAME + 1];

  // string sent to the MQTT_TOPIC_NOTIFY topic
  char connectStr[MAXLEN_DEVICE_IPSTR + MAXLEN_DEVICE_NAME + 1];

  void ConnectWiFi(void);   // waits for connection to WiFi
  bool ConnectMqtt(void);   // returns True if now connected
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

  if (mqttClient.connected())
  {
    mqttClient.publish(MQTT_TOPIC_NOTIFY, connectStr, false); // don't retain
    return true;
  }

  DBGOUT(("Connect to Mqtt..."));
  if (mqttClient.connect(deviceName))
  {
    DBGOUT(("Subscribing to: %s", deviceName));
    mqttClient.subscribe(deviceName);
    return true;
  }

  DBGOUT(("Mqtt State: %s", mqttClient.state()));
  BlinkStatusLED(0, 1);
  return false;
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

    wifiMQTT.replyString[0] = 0;

    if (*cmdStr == '?')
    {
      // command to retrive info
      pAppCmd->execCmd(cmdStr);
      wifiMQTT.mqttClient.publish(MQTT_TOPIC_REPLY, wifiMQTT.replyString);
    }
    // all other PixelNut commands, without a reply
    else if (pAppCmd->execCmd(cmdStr)) CheckExecCmd(cmdStr);
  }
  else
  {
    DBGOUT(("MQTT message too long: %d bytes", msglen));
  }
}

void WiFiMqtt::setup(void)
{
  FlashGetName(deviceName);

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
  mqttClient.setServer(MQTT_CREDS_IPADDR, MQTT_CREDS_PORT);
  mqttClient.setCallback(CallbackMqtt);
  ConnectMqtt();

  const char *ipstr = WiFi.localIP().toString().c_str();

  DBGOUT(("Device \"%s\":", deviceName));
  DBGOUT(("  LocalIP=%s", ipstr));
  DBGOUT(("  Broker=%s:%d", MQTT_CREDS_IPADDR, MQTT_CREDS_PORT));
  DBGOUT(("  MaxBufSize=%d", MQTT_MAX_PACKET_SIZE));
  DBGOUT(("  KeepAliveSecs=%d", MQTT_KEEPALIVE));

  strcpy(connectStr, deviceName);
  strcat(connectStr, " ");
  strcat(connectStr, ipstr);

  DBGOUT(("---------------------------------------"));
}

bool WiFiMqtt::setName(char *name)
{
  FlashSetName(name);
  return true;
}

bool WiFiMqtt::sendReply(char *instr)
{
  DBGOUT(("ReplyStr: \"%s\"", instr));
  strcat(replyString, instr);
  strcat(replyString, "\r\n");
  return true;
}

void WiFiMqtt::loop(void)
{
    if (ConnectMqtt()) mqttClient.loop();
}

#endif // MQTT_ESP32
//========================================================================================
