/*
  ArduinoMqttClient - Relay control with OTA

  This example connects to a MQTT broker, listens for incomming message ans cotrol Relay accordingly.
  Also sends confirmation to sending client on relay status.

  The circuit:
  - ESP01S with relay shield.
  Board LOLIN WEMOS D1 mini lite FS 64KB OTA ~~470KB

  This example code is in the public domain.
*/
//#define OTA
#include <ArduinoMqttClient.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
#include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
#include <WiFi101.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif
#if defined OTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#endif
//#define STATIONID "SP5IOU2-RELAY"
//#define OTAPASS "sp5iou2"
//#define STATIONID "SP5IOU3-RELAY"
//#define OTAPASS "sp5iou3"
#define STATIONID "SP5IOU4-RELAY"
#define OTAPASS "sp5iou4"
#define RELAY 0 //Relay port
//#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
//char ssid[] = SECRET_SSID;    // your network SSID (name)
//char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
#include<ssid_pass_SP5IOU.h> //Tis condists of WiFi SSID and password.
char ssid[] = STASSID;    // your network SSID (name)
char pass[] = STAPASS;    // your network password (use for WPA, or use as key for WEP)
#include <mqtt_credentials_SP5IOU.h> //File in libraries with credentials for mqtt broker consist of:
//#define CLIENTID "me broker user"  and #define CLIENTPASS "my broker password"

// To connect with SSL/TLS:
// 1) Change WiFiClient to WiFiSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate
//    flashed in the WiFi module.

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

//const char broker[] = "test.mosquitto.org";
//const char broker[] = "sp5iou-ds120j.home";
//const char broker[] = "sp5iou.freeddns.org";
const char broker[] = "192.168.1.120";
//int        port     = 1883;
int        port     = 1888;
//const char topic[]  = "arduino/simple";
//const char topic[]  = "testTopic";
const char topic[] = "Relay";

int count = 0;

//Function delayNoHang. Use only in infinite loop. Do not use in setup() it will not work.
boolean delayNoHang(unsigned long delayn) {
  static unsigned long previousMillis;
  unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis) >= delayn) {
    previousMillis = currentMillis;
    return true;
  } else {
    return false;
  }
}

void setup() {
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  //Initialize serial and wait for port to open:
  //  //Serial.begin(9600);
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to WiFi network:
  Serial.println();
  Serial.print(F("Attempting to connect to WPA SSID: "));
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    // failed, retry
    //Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
  }

  Serial.println(F("You're connected to the network: "));
  Serial.print(ssid);
  Serial.print(F(" IP address: "));
  Serial.println(WiFi.localIP());

  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  mqttClient.setId(STATIONID);

  // You can provide a username and password for authentication
  mqttClient.setUsernamePassword(CLIENTID, CLIENTPASS);

  Serial.print(F("Attempting to connect to the MQTT broker: "));
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    //Serial.print("MQTT connection failed! Error code = ");
    //Serial.println(mqttClient.connectError());

    //    while (1); //Infinite loop for WDT restart.
  }
  else {

    Serial.println(F("You're connected to the MQTT broker!"));
    Serial.println();
    Serial.print(F("Subscribing to topic: "));
    Serial.println(topic);
    Serial.println();
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);

    // subscribe to a topic
    mqttClient.subscribe(topic);

    // topics can be unsubscribed using:
    // mqttClient.unsubscribe(topic);

    Serial.print(F("Waiting for messages on topic: "));
    Serial.println(topic);
    Serial.println();
  }
  #if defined OTA
  //************ OTA SETUP ******************
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(STATIONID);

  // No authentication by default
  ArduinoOTA.setPassword(OTAPASS);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    }
    else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    //Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    //Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    //Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      //Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR) {
      //Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR) {
      //Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR) {
      //Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR) {
      //Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  //Serial.println("Ready");
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());

  //********* OTA SETUP END *****************
#endif
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  // mqttClient.poll();
  // to avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  int messageSize = mqttClient.parseMessage();
  String msgIn = "";
  if (messageSize) {
    digitalWrite(LED_BUILTIN, LOW);
//     we received a message, print out the topic and contents
    Serial.print(F("Received a message with topic '"));
    Serial.print(mqttClient.messageTopic());
    Serial.print(F("', length "));
    Serial.print(messageSize);
    Serial.println(F(" bytes:"));
//     use the Stream interface to print the contents
    while (mqttClient.available()) {
      msgIn += (char)mqttClient.read();
    Serial.print((char)mqttClient.read());
    }
    //Serial.println(msgIn);
    ////Serial.println();
    //Serial.println();
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
  }

  if (msgIn == ((String)STATIONID + " ON")) {
    digitalWrite(RELAY, LOW);
    Serial.print(F("Sending message to topic: "));
    Serial.println(topic);
    Serial.print((String)STATIONID + F(" IS ON"));
    //      //Serial.println(count);

    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print((String)STATIONID + F(" IS ON"));
    //mqttClient.print(count);
    mqttClient.endMessage();
    Serial.println();
  }
  else if (msgIn == ((String)STATIONID + " OFF")) {
    digitalWrite(RELAY, HIGH);
    Serial.print(F("Sending message to topic: "));
    Serial.println(topic);
    Serial.print((String)STATIONID + F(" IS OFF"));
    //          //Serial.println(count);

    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print((String)STATIONID + F(" IS OFF"));
    //mqttClient.print(count);
    mqttClient.endMessage();
    //Serial.println();
  }
  else if(msgIn.startsWith((String)STATIONID + F(" GATE OPEN"))) {
    String conditionString = (String)STATIONID + F(" GATE OPEN");
    Serial.println(conditionString);
    int onDelay = 15000; //Default gate open time is 15 sec.
    unsigned int conditionLength = conditionString.length();
    if(msgIn.length() > conditionLength){
     onDelay = msgIn.substring(conditionLength+1).toInt() * 1000;
     if(onDelay > 120000) onDelay = 120000; //Limits to 2 min
     //Serial.println(onDelay);
    }
    digitalWrite(RELAY, LOW); //Open lock
    Serial.print(F("Sending message to topic: "));
    Serial.println(topic);
    Serial.print((String)STATIONID + F(" GATE IS OPEN"));
    //          //Serial.println(count);
    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print((String)STATIONID + F(" GATE IS OPEN"));
    //mqttClient.print(count);
    mqttClient.endMessage();
    //Serial.println();
    delay(onDelay); //Lock is open for 15 sec.
    digitalWrite(RELAY, HIGH); // Close lock
    Serial.print(F("Sending message to topic: "));
    Serial.println(topic);
    Serial.print((String)STATIONID + F(" GATE IS CLOSED"));
    //          //Serial.println(count);
    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print((String)STATIONID + F(" GATE IS CLOSED"));
    //mqttClient.print(count);
    mqttClient.endMessage();
    Serial.println();
  }
  msgIn = "";
#if defined OTA
  ArduinoOTA.handle(); //It is for OTA
#endif
  if (!mqttClient.connected() && delayNoHang(60000)) {
    Serial.println(F("Connection with mqqt brioker lost. attepting to restart"));
      while (1); //If connection wth broiker is lost for any reason, loop infinitely to let WDT timer to restart after derfault 6s.
      //ESP.restart;
  }
}
