/*
Todo:
DONE: Touch button to acknowledge
- Sleep when there are no events in the next days
- Contact time server less often and maintain time internally (possible? => can we detect if the device returns from sleep or gets replugged?)
- Start Wifi only when required, not on start-up
- Wifi App to configure start and end time (15:00-8:00), what events to show
DONE: option to have parallel events (e.g. Paper & Bio on the same day => alternate between the different colors sequentially) => https://stackoverflow.com/questions/42701688/using-an-unordered-map-with-arrays-as-keys
- query API directly instead of hard-coded list
- Add magnets to trash can so it snapps in place
Helpful:
Epoch Converter: https://www.epochconverter.com/
*/

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#include "webpage.h"
#include "data_neuweiler.h"

int colorIds[] = { -1, -1, -1 };  //allow max three tasks on the same day
int colorIdsLast[] = { -1, -1, -1 };
int numberOfColorIds = sizeof(colorIds) / sizeof(colorIds[0]);
int colorIndex = 0;  //used to toggle between multiple colors for same day tasks

int startHour = 15;       //am Vortag
int endHour = 8;          //am Abholugstag
int brightness = 255;     //highest value since used to fadeBy...
int fadeAmount = 5;       // Set the amount to fade to 5, 10, 15, 20, 25 etc even up to 255.
int showDuration = 5000;  //ms Splash screen
unsigned long millisLast = 0;

#define STATE_INIT 0
#define STATE_SHOW 1
#define STATE_DISCONNECTED 2
#define STATE_QUERY 3
#define STATE_CONFIGURE 4

int STATE = STATE_INIT;
int STATE_PREVIOUS = -1;
int STATE_NEXT = -1;
String stateTbl[] = { "STATE_INIT", "STATE_SHOW", "STATE_DISCONNECTED", "STATE_QUERY", "STATE_CONFIGURE" };

#include <FastLED.h>
#define NUM_LEDS 1
#define DATA_PIN 4  //D2
#define BRIGHTNESS 0
#define FRAMES_PER_SECOND 120
CRGB leds[NUM_LEDS];
uint8_t gHue = 0;  // rotating "base color" used by many of the patterns

//Switch
#include <Button.h>
#define REED_PIN 5  //D1
Button reed(REED_PIN, LOW, BTN_PULLUP, LATCHING);
int switchCounter = 0;
int multiClickTimeout = 1000;  //ms
unsigned long lastSwitchMillis = 0;

int acknowledge = 0;   //turn off reminder light when the Mülleimer is lupft
int triggerEpoch = 0;  //used to detect if the epochDict is changing => reset acknowledge
int initialized = 0;   //in order to prevent acknowledge to be triggered at the beginning

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
int nowEpoch = 0;            //global since only querying every minute
int queryIntervall = 60000;  //ms => every minute (could be less, however to really turn on LED at intended time...)
unsigned long lastQueryMillis = 0;

void setup() {
  Serial.begin(115200);
  reed.attachEdgeDetect(doNothing, setAcknowledge);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
  leds[0] = CRGB::Red;                                      //in case no successful WiFi connection
  FastLED.show();
  WiFiManager wm;
  //  wm.resetSettings();
  if (wm.autoConnect("TrashReminder")) {
    Serial.println("Successfully connected.");
  } else {
    Serial.println("Failed to connect.");
  }
  WiFi.hostname("TrashReminder");
  //Time Client
  timeClient.begin();
  timeClient.setTimeOffset(3600);  //GMT+1
}

void loop() {
  handleState();
}

// ESP Functions
void handleConnection() {
  if (!WiFi.isConnected()) {
    Serial.println("Connection lost!");
    STATE_NEXT = STATE_DISCONNECTED;
  }
}

int getCurrentTimeEpoch() {
  timeClient.update();
  return (timeClient.getEpochTime());
}

void resetColorIds() {
  memset(colorIds, -1, sizeof(colorIds));
}

void printColorIds() {
  for (int i = 0; i < sizeof colorIds / sizeof colorIds[0]; i++) {
    Serial.println("colorIds[" + String(i) + "] = " + String(colorIds[i]));
  }
}
void handleLed(int nowEpoch) {
  int dictEpoch;
  resetColorIds();
  //  printColorIds();
  for (auto entry : epochTaskDict) {
    dictEpoch = entry.first;
    //    Serial.println("now: " + String(nowEpoch) + ", dictEpoch = " + String(dictEpoch) + ", dictEpoch+startTime = " + String(dictEpoch+startTime)+ ", dictEpoch+endTime = " + String(dictEpoch+endTime));
    if ((nowEpoch > dictEpoch + (startHour - 24) * 60 * 60) && (nowEpoch < dictEpoch + endHour * 60 * 60)) {
      if (dictEpoch != triggerEpoch) {
        acknowledge = 0;  //acknowledge only valid for same triggerEpoch
        colorIndex = 0;
        Serial.println("Resetting since new trigger!");
      }
      triggerEpoch = dictEpoch;
      if (!acknowledge) {
        setColorIds(entry.second);
      }
    }
  }
  //  printColorIds();
  setTaskColor();
  FastLED.show();
}

boolean isValidTask(int taskId) {
  for (int i = 0; i < sizeof(validIndex) / sizeof(validIndex[0]); i++) {
    if (taskId == validIndex[i]) { return (true); }
  }
  return (false);
}

void setColorIds(String taskIds) {
  int start = 0, index = 0, indexFound, taskId;
  do {
    indexFound = taskIds.indexOf(",", start);
    taskId = taskIds.substring(start, indexFound).toInt();
    if (isValidTask(taskId)) { colorIds[index++] = taskId; }
    Serial.println("taskId = " + String(taskId) + ", start = " + String(start) + ", indexFound = " + String(indexFound) + ", validTask = " + String(isValidTask(taskId)));
    start = indexFound + 1;
  } while (indexFound != -1);
}

void handleReed() {
  reed.update();
  initialized = true;
}

void setAcknowledge() {
  unsigned long now = millis();
  if (initialized) {
    //    Serial.println("OFF - Acknowedge!");
    acknowledge = 1;
    //Multi-click detection
    switchCounter++;
    if (((now - lastSwitchMillis) > multiClickTimeout) || (lastSwitchMillis == 0)) { switchCounter = 0; }
    //    Serial.println("Counter = " + String(switchCounter));
    lastSwitchMillis = now;
    if (switchCounter == 2) {
      if (STATE == STATE_CONFIGURE) {
        STATE_NEXT = STATE_INIT;  //reset
      } else {
        STATE_NEXT = STATE_CONFIGURE;
      }
    }
  }
}

void doNothing() {
  //  Serial.println("ON");
  true;
}

void incrementColorId() {
  int numberOfTasks = 0;
  for (int i = 0; i < numberOfColorIds; i++) {
    if (colorIds[i] != -1) { numberOfTasks++; }
  }
  colorIndex++;
  if (colorIndex >= numberOfTasks) { colorIndex = 0; }
  //  Serial.println("colorIndex = " + String(colorIndex) + ", numberOfTasks = " + String(numberOfTasks) + ", numberOfColorIds = " + String(numberOfColorIds));
}

void setBrightness(int blinkSpeed = 20, boolean reset = false) {
  if (reset) {
    brightness = 255;
  } else {
    if ((colorIds[0] != colorIdsLast[0]) || (colorIds[1] != colorIdsLast[1])) { brightness = 255; }
    colorIdsLast[0] = colorIds[0];
    colorIdsLast[1] = colorIds[1];
  }
  leds[0].fadeLightBy(brightness);
  brightness = brightness + fadeAmount;
  if (brightness <= 0) { brightness = 0; }
  if (brightness >= 255) {
    brightness = 255;
    incrementColorId();
  }
  //    Serial.println("Brightness = " + String(brightness) + ", ColorIndex = " + String(taskId));
  if (brightness <= 0 || brightness >= 255) { fadeAmount = -fadeAmount; }
  delay(blinkSpeed);
}

void setTaskColor() {
  if (colorIds[0] == -1) {
    //    Serial.println("No Event");
    leds[0] = CRGB::Black;
  } else {
    // EVENT
    //    Serial.println("Event: " + task[taskId] + " => LED = " + String(color[taskId],HEX));
    leds[0] = color[colorIds[colorIndex]];
    setBrightness();
  }
}

void setColor(int color, boolean fade = true, int blinkSpeed = 20) {
  leds[0] = color;
  if (fade) { setBrightness(blinkSpeed); }
  FastLED.show();
}

/// Splash Screen
void rainbow() {
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() {
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(50);
}

void addGlitter(fract8 chanceOfGlitter) {
  if (random8() < chanceOfGlitter) {
    leds[random16(NUM_LEDS)] += CRGB::White;
  }
}

// Server Functions
void startWebServer() {
  Serial.println("Starting WebServer...");
  server.on("/", handleRoot);
  server.on("/set_start", setStartHour);
  server.on("/set_end", setEndHour);
  server.on("/settings", sendSettings);
  server.onNotFound(notFound);
  server.begin();
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print(", hostName: ");
  Serial.println(WiFi.getHostname());
}

void sendSettings() {
  String value;
  value = String(startHour) + "," + String(endHour);
  Serial.println("Sending settings: " + value);
  server.send(200, "text/plane", value);
}

void stopWebServer() {
  Serial.println("Stopping WebServer...");
  server.stop();
}

void notFound() {
  server.send(404, "text/plain", "Not found");
}

void handleRoot() {
  String s = webpage;
  server.send(200, "text/html", s);
}

void setStartHour() {
  String hour = server.arg("value");
  Serial.println("Setting startHour = " + hour);
  startHour = hour.toInt();
  setColor(CRGB::White, false);
  setColor(CRGB::Purple, false);
}

void setEndHour() {
  String hour = server.arg("value");
  Serial.println("Setting endHour = " + hour);
  endHour = hour.toInt();
  setColor(CRGB::White, false);
  setColor(CRGB::Purple, false);
}

void handleState() {
  unsigned long millisNow = millis();
  STATE_NEXT = -1;
  //  Serial.println("STATE = " + stateTbl[STATE] + ", STATE_PREVIOUS = " + stateTbl[STATE_PREVIOUS]);
  if (STATE != STATE_PREVIOUS) { Serial.println("STATE = " + stateTbl[STATE]); }
  switch (STATE) {
    case STATE_INIT:  //***********************************************************
      if (STATE_PREVIOUS == STATE_CONFIGURE) { stopWebServer(); }
      millisLast = millisNow;
      brightness = 255;
      colorIndex = 0;
      acknowledge = 0;
      resetColorIds();
      STATE_NEXT = STATE_SHOW;
      break;
    case STATE_SHOW:  //***********************************************************
      rainbowWithGlitter();
      FastLED.show();
      // insert a delay to keep the framerate modest
      FastLED.delay(1000 / FRAMES_PER_SECOND);
      // do some periodic updates
      EVERY_N_MILLISECONDS(20) {
        gHue++;
      }  // slowly cycle the "base color" through the rainbow
      if ((millisNow - millisLast) > showDuration) { STATE_NEXT = STATE_QUERY; }
      break;
    case STATE_DISCONNECTED:  //***********************************************************
      setColor(CRGB::Red, true, 2);
      if (WiFi.isConnected()) { STATE_NEXT = STATE_INIT; }
      break;
    case STATE_QUERY:  //***********************************************************
      if (((millisNow - lastQueryMillis) > queryIntervall) || (lastQueryMillis == 0)) {
        nowEpoch = getCurrentTimeEpoch();
        Serial.println("Received current epoch time: " + String(nowEpoch));
        lastQueryMillis = millisNow;
      }
      handleLed(nowEpoch);
      handleReed();
      handleConnection();
      break;
    case STATE_CONFIGURE:
      if (STATE_PREVIOUS == STATE_QUERY) { startWebServer(); }
      setColor(CRGB::Purple, false);
      server.handleClient();
      handleReed();
      break;
    default:
      break;
  }
  STATE_PREVIOUS = STATE;
  if (STATE_NEXT != -1) { STATE = STATE_NEXT; }
}