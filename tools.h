
#define relStateOFF LOW
#define relStateON HIGH
#define LEDStateOFF HIGH
#define LEDStateON LOW
#define butStateOFF HIGH
#define butStateON LOW

const int CMD_WAIT = 0;
const int CMD_BUTTON_CHANGE = 1;
int cmd = CMD_WAIT;

int relayState = relStateOFF;

//inverted button state
int buttonState = HIGH;

static long startPress = 0;

void setState(int s) {

  relayState = s;

  DebugPrintln(relayState);

  digitalWrite(SONOFF_RELAY, relayState);
  if (relayState == relStateOFF) {
    //    digitalWrite(SONOFF_LED, LEDStateOFF);
    client.publish(mqtt_pubtopic_rl, "off", true);
  }
  else {
    //    digitalWrite(SONOFF_LED, LEDStateON);
    client.publish(mqtt_pubtopic_rl, "on", true);
  }


}

void turnOn() {

  setState(relStateON);
}

void turnOff() {

  setState(relStateOFF);
}

void toggleState() {
  cmd = CMD_BUTTON_CHANGE;
}

void toggle() {
  DebugPrintln("toggle state");
  relayState = relayState == relStateOFF ? relStateON : relStateOFF;
  setState(relayState);
}

void restart() {
  ESP.reset();
  delay(1000);
}

void reset() {
  //reset settings to defaults
  /*
    WMSettings defaults;
    settings = defaults;
    EEPROM.begin(512);
    EEPROM.put(0, settings);
    EEPROM.end();
  */
  //reset wifi credentials
  WiFi.disconnect();
  delay(1000);
  ESP.reset();
  delay(1000);
}

void init_tools() {
  // initialize the pushbutton pin as an input:
  pinMode(SONOFF_BUTTON, INPUT);

  //set relay pin as output
  pinMode(SONOFF_RELAY, OUTPUT);

  //set led pin as output
  pinMode(SONOFF_LED, OUTPUT);
}

void check_button() {
  switch (cmd) {
    case CMD_WAIT:
      break;
    case CMD_BUTTON_CHANGE:
      int currentState = digitalRead(SONOFF_BUTTON);
      if (currentState != buttonState) {
        if (buttonState == butStateON && currentState == butStateOFF) {
          long duration = millis() - startPress;
          if (duration < 10) {
            DebugPrintln("too short press - no action");
          } else if (duration < 5000) {
            DebugPrintln("short press - toggle relay");
            toggle();
          } else if (duration < 10000) {
            DebugPrintln("medium press - reset");
            restart();
          } else if (duration < 60000) {
            DebugPrintln("long press - reset settings");
            reset();
          }
        } else if (buttonState == butStateOFF && currentState == butStateON) {
          startPress = millis();
        }
        buttonState = currentState;
      }
      break;
  }
}

