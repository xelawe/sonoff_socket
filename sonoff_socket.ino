/*
    Flash Mode: DIO
    Flash Frequency: 40MHz
    Upload Using: Serial
    CPU Frequency: 80MHz
    Flash Size: 1M (64K SPIFFS)
    Debug Port: Disabled
    Debug Level: None
    Reset Method: ck
    Upload Speed: 115200
    Port: Your COM port connected to sonoff

   1MB flash size

   sonoff header
   1 - vcc 3v3
   2 - rx
   3 - tx
   4 - gnd
   5 - gpio 14

   esp8266 connections
   gpio  0 - button
   gpio 12 - relay
   gpio 13 - green led - active low
   gpio 14 - pin 5 on header

*/
#include <cy_serdebug.h>
#include <cy_serial.h>

#include "cy_wifi.h"
#include "cy_ota.h"
#include "cy_mqtt.h"
#include <Ticker.h>

const char* gc_hostname = "SonofSo1";
const char* mqtt_pubtopic_rl = "ATSH28/UG/Z3/RL/1/state";
const char* mqtt_subtopic_rl = "ATSH28/UG/Z3/RL/1/set";

#define SONOFF_BUTTON    0
#define SONOFF_RELAY    12
#define SONOFF_LED      13

#include "tools.h"

Ticker ticker;

void tick()
{
  //toggle LED state
  int state = digitalRead(SONOFF_LED);  // get the current state of GPIO1 pin
  digitalWrite(SONOFF_LED, !state);     // set pin to the opposite state
}


void callback_mqtt1(char* topic, byte* payload, unsigned int length) {
  DebugPrintln("Callback 1 - Set Relay");

  String message_string = "";

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    //fill up the message string
    message_string.concat((char)payload[i]);
  }
  Serial.println();
    
  //map payload / commands
  int shutter_cmd = 0;
  if (message_string.equalsIgnoreCase("on")) {
    turnOn();
  }
  else if (message_string.equalsIgnoreCase("off")) {
    turnOff();
  }
  else if (message_string.equalsIgnoreCase("toggle")) {
    toggle();
  }
  else {
    Serial.print("Received illegal command message: ");
    Serial.println(message_string.c_str());
  }
}

void setup() {
  cy_serial::start(__FILE__);

  init_tools();

  turnOff();

  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  wifi_init(gc_hostname);

  init_ota(gc_hostname);

  attachInterrupt(SONOFF_BUTTON, toggleState, CHANGE);

  ticker.detach( );
  turnOff();

  init_mqtt(gc_hostname);
  add_subtopic(mqtt_subtopic_rl, callback_mqtt1);
}

void loop() {

  check_ota();
  
  check_mqtt();

  check_button();

}

