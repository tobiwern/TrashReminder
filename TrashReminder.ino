/*
Todo:
DONE: Touch button to acknowledge
- Sleep when there are no events in the next days
- Contact time server less often and maintain time internally (possible? => can we detect if the device returns from sleep or gets replugged?)
- Start Wifi only when required, not on start-up
DONE: Wifi App to configure start and end time (15:00-8:00), what events to show
DONE: option to have parallel events (e.g. Paper & Bio on the same day => alternate between the different colors sequentially) => https://stackoverflow.com/questions/42701688/using-an-unordered-map-with-arrays-as-keys
- query API directly instead of hard-coded list
DONE: javascript: Translate dateDict filtering with TrashType => writing to LittleFS
DONE: store default values to EEPROM (start/endHour)
- check if there are more tasksPerDay then allowed
- Add magnets to trashcan so it snapps in place
- reset STATE_CONFIG after a timeout of inactivity
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

//forward function prototypes (so function order does not matter)
//void setColor(int color, boolean fade, int blinkSpeed);

const int maxNumberOfTasksPerDay = 4;
int colorIds[maxNumberOfTasksPerDay];
int colorIdsLast[maxNumberOfTasksPerDay];
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

int acknowledge = 0;   //turn off reminder light when the M??lleimer is lupft
int triggerEpoch = 0;  //used to detect if the epochDict is changing => reset acknowledge
int initialized = 0;   //in order to prevent acknowledge to be triggered at the beginning

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
unsigned int nowEpoch = 0;   //global since only querying every minute
int queryIntervall = 60000;  //ms => every minute (could be less, however to really turn on LED at intended time...)
unsigned long lastQueryMillis = 0;

//data
const char* dataFile = "/data.json";
const char* settingsFile = "/settings.json";
int numberOfValidTaskIds = 0;  //global counter
int numberOfTaskIds = 0;       //global counter
int numberOfEpochs = 0;        //global counter
const int maxNumberOfEpochs = 200; 
const int maxNumberOfTaskIds = 20;

String task[maxNumberOfTaskIds];
int color[maxNumberOfTaskIds];
int validTaskId[maxNumberOfTaskIds];

typedef struct epochTask {
  unsigned int epoch;
  int taskIds[maxNumberOfTasksPerDay];
} epochTask;

epochTask epochTaskDict[maxNumberOfEpochs];

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
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

void printColorIds() {  //debug
  for (int i = 0; i < maxNumberOfTasksPerDay; i++) {
    Serial.println("colorIds[" + String(i) + "] = " + String(colorIds[i]));
  }
}

void handleLed(unsigned int nowEpoch) {
  int dictEpoch;
  memset(colorIds, -1, sizeof(colorIds));
  //  printColorIds();
  for (int i = 0; i < numberOfEpochs; i++) {
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
  //printColorIds();
  setTaskColor();
  FastLED.show();
}

boolean isValidTask(int taskId) {
  for (int i = 0; i < numberOfValidTaskIds; i++) {
    if (taskId == validTaskId[i]) { return (true); }
  }
  return (false);
}

void setColorIds(int taskIds[]) {
  int index = 0;
  //Serial.println("==================");
  for (int i = 0; i < maxNumberOfTasksPerDay; i++) {
    //Serial.println("taskId[" + String(i) + "] = " + String(int(taskIds[i])) + ", valid = " + String(isValidTask(taskIds[i])));
    if (taskIds[i] != -1) {
      if (isValidTask(taskIds[i])) { colorIds[index++] = int(taskIds[i]); }
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
    Serial.println("OFF - Acknowedge!");
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
  for (int i = 0; i < maxNumberOfTasksPerDay; i++) {  //detect how many tasks
    if (colorIds[i] != -1) { numberOfTasks++; }
  }
  colorIndex++;
  if (colorIndex >= numberOfTasks) { colorIndex = 0; }
  //  Serial.println("colorIndex = " + String(colorIndex) + ", numberOfTasks = " + String(numberOfTasks) + ", maxNumberOfTasksPerDay = " + String(maxNumberOfTasksPerDay));
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

/* //DEBUG
boolean initDataFromFile() {
  numberOfValidTaskIds = 4;
  validTaskId[0] = 0;
  validTaskId[1] = 1;
  validTaskId[2] = 2;
  validTaskId[3] = 3;
  numberOfTaskIds = 4;
  task[0] = "Biom??ll";
  task[1] = "Papier";
  task[2] = "Restm??ll";
  task[3] = "Wertstoffe";
  color[0] = 0x00FF00;
  color[1] = 0x0000FF;
  color[2] = 0xFFFFFF;
  color[3] = 0xFFFF00;
  numberOfEpochs = 2;
  epochTask entry0 = {.epoch = 1674000000, .taskIds = {2,1,-1,-1} };
  epochTaskDict[0] = entry0;
  epochTask entry1 = {.epoch = 1674518400, .taskIds = {0,-1,-1,-1} };
  epochTaskDict[1] = entry1;
  return (true);
}
*/

void handleState() {
  unsigned long millisNow = millis();
  STATE_NEXT = -1;
 // Serial.println("STATE = " + stateTbl[STATE] + ", STATE_PREVIOUS = " + stateTbl[STATE_PREVIOUS]);
  if (STATE != STATE_PREVIOUS) { Serial.println("STATE = " + stateTbl[STATE]); }
  switch (STATE) {
    case STATE_INIT:  //***********************************************************
      if (STATE_PREVIOUS == STATE_CONFIGURE) { stopWebServer(); }
      millisLast = millisNow;
      brightness = 255;
      colorIndex = 0;
      acknowledge = 0;
      memset(colorIds, -1, sizeof(colorIds));
      memset(colorIdsLast, -1, sizeof(colorIdsLast));
//      listDir("/"); //ToDo1
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
      if (STATE_PREVIOUS == STATE_QUERY) {
        startWebServer();
      }
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
