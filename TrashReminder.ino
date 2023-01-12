/*
Todo:
DONE: Touch button to acknowledge
- Sleep when there are no events in the next days
- Contact time server less often and maintain time internally (possible? => can we detect if the device returns from sleep or gets replugged?)
- Start Wifi only when required, not on start-up
DONE: Wifi App to configure start and end time (15:00-8:00), what events to show
DONE: option to have parallel events (e.g. Paper & Bio on the same day => alternate between the different colors sequentially) => https://stackoverflow.com/questions/42701688/using-an-unordered-map-with-arrays-as-keys
- query API directly instead of hard-coded list
- Add magnets to trash can so it snapps in place
- javascript: Translate dateDict filtering with TrashType => writing to LittleFS
- store default values to EEPROM (start/endHour)
Helpful:
Epoch Converter: https://www.epochconverter.com/
JSON Validator: https://jsonformatter.curiousconcept.com/#
*/

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager

#include "filesystem.h"
#include "webpage.h"
#include "data_neuweiler.h"

//forward function prototypes (so function order does not matter)
//void setColor(int color, boolean fade, int blinkSpeed);

const int maxNumberOfTaskIds = 3; //allow max three different tasks per day
int colorIds[maxNumberOfTaskIds] = { -1, -1, -1 };  //better use memset(colorIds, -1, sizeof(colorIds)); ?
int colorIdsLast[maxNumberOfTaskIds] = { -1, -1, -1 };
int colorIndex = 0;  //used to toggle between multiple colors for same day tasks

int startHour = 15;       //am Vortag
int endHour = 8;          //am Abholugstag
int brightness = 255;     //highest value since used to fadeBy...
int fadeAmount = 5;       // Set the amount to fade to 5, 10, 15, 20, 25 etc even up to 255.
int showDuration = 5000;  //ms Splash screen
unsigned long millisLast = 0;

/// STATE MACHINE
#define STATE_INIT 0
#define STATE_SHOW 1
#define STATE_DISCONNECTED 2
#define STATE_QUERY 3
#define STATE_CONFIGURE 4

int STATE = STATE_INIT;
int STATE_PREVIOUS = -1;
int STATE_NEXT = -1;
int STATE_FOLLOWING = -1;
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

//data
const char* dataFile = "/data.json";
const char* settingsFile = "/settings.json";
int numberOfColors = 0;
int numberOfValidTaskIds = 0; //global counter 
int numberOfTaskIds = 0;
int numberOfEpochs = 0;
String task[4];
int color[4];
int validIndex[4];  //ToDo 20

typedef struct epochTask {
  unsigned int epoch;
  unsigned char taskIds[maxNumberOfTaskIds];  
} epochTask;

epochTask epochTaskDict[100];

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
  //  deleteFile(dataFile);
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

void printColorIds() { //debug
  for (int i = 0; i < numberOfColors; i++) {
    Serial.println("colorIds[" + String(i) + "] = " + String(colorIds[i]));
  }
}

void handleLed(int nowEpoch) {
  int dictEpoch;
  resetColorIds();
  //  printColorIds();
  for (int i = 0; i < numberOfEpochs; i++) {
    //for (auto entry : epochTaskDict) {
    //  dictEpoch = entry.first;
    dictEpoch = epochTaskDict[i].epoch;
    if ((nowEpoch > dictEpoch + (startHour - 24) * 60 * 60) && (nowEpoch < dictEpoch + endHour * 60 * 60)) {
      //      Serial.println("nowEpoch: " + String(nowEpoch) + ", dictEpoch = " + String(dictEpoch) + ", dictEpoch+startTime = " + String(dictEpoch + (startHour - 24) * 60 * 60) + ", dictEpoch+endTime = " + String(dictEpoch + endHour * 60 * 60));
      if (dictEpoch != triggerEpoch) {
        acknowledge = 0;  //acknowledge only valid for same triggerEpoch
        colorIndex = 0;
        Serial.println("Resetting since new trigger!");
      }
      triggerEpoch = dictEpoch;
      if (!acknowledge) {
        setColorIds(epochTaskDict[i].taskIds);
      }
    }
  }
  //  printColorIds();
  setTaskColor();
  FastLED.show();
}

boolean isValidTask(int taskId) {
  for (int i = 0; i < numberOfValidTaskIds; i++) {
    if (taskId == validIndex[i]) { return (true); }
  }
  return (false);
}

void setColorIds(unsigned char taskIds[]) {
  int index = 0;
  for (int i = 0; i < maxNumberOfTaskIds; i++) {
    if (taskIds[i] != -1) {
      if (isValidTask(taskIds[i])) { colorIds[index++] = taskIds[i]; }
    }
  }
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
  for (int i = 0; i < maxNumberOfTaskIds; i++) { //detect how many tasks
    if (colorIds[i] != -1) { numberOfTasks++; }
  }
  colorIndex++;
  if (colorIndex >= numberOfTasks) { colorIndex = 0; }
  //  Serial.println("colorIndex = " + String(colorIndex) + ", numberOfTasks = " + String(numberOfTasks) + ", maxNumberOfTaskIds = " + String(maxNumberOfTaskIds));
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

#include "webserver.h"  //separate file for webserver functions

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
      listDir("/");
      initStartEndTimes();  //initializes startHour and endHour
      initDataFromFile();
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
      if ((millisNow - millisLast) > showDuration) {
        if (STATE_FOLLOWING != -1) {
          STATE_NEXT = STATE_FOLLOWING;
          STATE_FOLLOWING = -1;
        } else {
          STATE_NEXT = STATE_QUERY;
        }
      }
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
