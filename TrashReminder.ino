/*
Todo:
- Touch button to acknowledge
- Sleep when there are no events in the next days
- Contact time server less often and maintain time internally (possible? => can we detect if the device returns from sleep or gets replugged?)
- Start Wifi only when required, not on start-up
- Wifi App to configure start and end time (15:00-8:00), what events to show
- option to have parallel events (e.g. Paper & Bio on the same day => alternate between the different colors sequentially) => https://stackoverflow.com/questions/42701688/using-an-unordered-map-with-arrays-as-keys
- query API directly instead of hard-coded list
*/

#include <unordered_map>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <credentials.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

std::unordered_map<int,int>epochTaskDict = {
{1685750400,3},
{1686873600,3},
{1695600000,0},
{1682035200,3},
{1685923200,0},
{1699833600,1},
{1701043200,1},
{1692921600,3},
{1678406400,3},
{1703894400,3},
{1681171200,0},
{1673827200,0},
{1676851200,1},
{1674777600,3},
{1682985600,1},
{1695340800,3},
{1690761600,0},
{1690156800,1},
{1679270400,1},
{1687737600,1},
{1673222400,1},
{1683504000,0},
{1688947200,1},
{1674432000,1},
{1693180800,0},
{1702252800,1},
{1673568000,3},
{1675641600,1},
{1684540800,3},
{1678060800,1},
{1688342400,0},
{1698624000,1},
{1681689600,1},
{1694131200,3},
{1679616000,3},
{1688083200,3},
{1689292800,3},
{1699056000,3},
{1691712000,3},
{1701388800,3},
{1680912000,3},
{1677196800,3},
{1702857600,0},
{1683331200,3},
{1690502400,3},
{1702598400,3},
{1680480000,1},
{1676246400,0},
{1698969600,2},
{1697414400,1},
{1693785600,1},
{1703289600,1},
{1700179200,3},
{1675987200,3},
{1696636800,3},
{1697760000,3},
{1680134400,2},
{1692576000,1},
{1698019200,0},
{1694995200,1},
{1700438400,0},
{1696204800,1},
{1678665600,0},
{1686528000,1},
{1684108800,1},
{1685404800,1},
{1691366400,1},
{1672099200,1}, //2022.12.27
{1672444800,3}, //2022.12.31
{1671753600,3}, //dummy
{1671840000,2}, //dummy
};
const String task[] = {"Altpapier-Tonne in Hirrlingen", "Gelber Sack in Hirrlingen", "Häckselgut in Hirrlingen", "Restmüll in Hirrlingen"};
//         yellow, green, white
const int color[] = {0x0000FF, 0xFFFF00, 0x00FF00, 0xFFFFFF };
const int validIndex[] = {0,1,2,3};
int startTime = 11*60*60; //seconds => 15 Uhr
int endTime = (24+8)*60*60; //seconds => nächster Tag 8 Uhr morgens
int taskIdLast = -1;
int brightness = 0;
int fadeAmount = 5;  // Set the amount to fade I usually do 5, 10, 15, 20, 25 etc even up to 255.
int showDuration = 5000; //ms
unsigned long millisLast = 0;

#define STATE_INIT 0
#define STATE_SHOW 1
#define STATE_DISCONNECTED 2
#define STATE_QUERY 3
#define STATE_CONFIGURE 4

int STATE = STATE_INIT;
int STATE_PREVIOUS = -1;
String stateTbl[] = {"STATE_INIT", "STATE_SHOW", "STATE_DISCONNECTED", "STATE_QUERY", "STATE_CONFIGURE"};

#include <FastLED.h>
#define NUM_LEDS 1
#define DATA_PIN 4 //D2
#define BRIGHTNESS         254
#define FRAMES_PER_SECOND  120
CRGB leds[NUM_LEDS];
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

//Switch
#include <Button.h>
#define REED_PIN 5 //D1
Button reed(REED_PIN, LOW, BTN_PULLUP, LATCHING);
int switchCounter = 0;
int multiClickTimeout = 1000; //ms
unsigned long lastSwitchMillis = 0;

int acknowledge = 0;
int triggerEpoch = 0;
int initialized = 0; //in order to prevent acknowledge to be triggered at the beginning

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
int nowEpoch = 0;
int queryIntervall = 60000; //ms => every minute (could be less, however to really turn on LED at intended time...) 
unsigned long lastQueryMillis = 0;
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  reed.attachEdgeDetect(doNothing, setAcknowledge);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
  leds[0] = CRGB::Red;
  FastLED.show();
  WiFi.hostname("TrashReminder");
  WiFiManager wm; 
//   wm.resetSettings();
   if(wm.autoConnect("TrashReminder")){
     Serial.println("Successfully connected.");     
   } else {
     Serial.println("Failed to connect.");
   }
  timeClient.begin();
  timeClient.setTimeOffset(3600); //GMT+1
}

void loop() {
  handleState();
}

void handleState(){
  unsigned long millisNow = millis();
  if(STATE != STATE_PREVIOUS) {Serial.println("STATE = " + stateTbl[STATE]);}
  switch (STATE) {
    case STATE_INIT:                //***********************************************************
      millisLast = millisNow;
      brightness = 0;
      acknowledge = 0;
      STATE = STATE_SHOW;
      break;
    case STATE_SHOW:                //***********************************************************
      rainbowWithGlitter();
      FastLED.show();  
      // insert a delay to keep the framerate modest
      FastLED.delay(1000/FRAMES_PER_SECOND); 
      // do some periodic updates
      EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
      if((millisNow-millisLast) > showDuration){STATE = STATE_QUERY;}
      break;
    case STATE_DISCONNECTED:        //***********************************************************
      setFadeColor(CRGB::Red);
      if(WiFi.isConnected()){STATE = STATE_INIT;}
      break;  
    case STATE_QUERY:               //***********************************************************
      if(((millisNow-lastQueryMillis) > queryIntervall) || (lastQueryMillis == 0)){
        nowEpoch = getCurrentTimeEpoch();
        Serial.println("Received current epoch time: " + String(nowEpoch));
        lastQueryMillis = millisNow;
      } 
      handleLed(nowEpoch); 
      handleReed();
      handleConnection();
//      testLeds();
      break; 
    case STATE_CONFIGURE:
      setFadeColor(CRGB::Purple);
      handleReed();
      break;       
    default:
      break;
  }   
  STATE_PREVIOUS = STATE; 
}

void setFadeColor(int color){
  leds[0] = color;
  setBrightness(color); //usually index
  FastLED.show();
}

void handleConnection(){
  if(!WiFi.isConnected()){
    Serial.println("Connection lost!");
    STATE = STATE_DISCONNECTED;
  } 
}

int getCurrentTimeEpoch(){
  timeClient.update();
  return(timeClient.getEpochTime());
}

void handleLed(int nowEpoch){
  int dictEpoch;
  int taskId = -1;
  for (auto entry :epochTaskDict)  {
    dictEpoch = entry.first;
//    Serial.println("now: " + String(nowEpoch) + ", dictEpoch = " + String(dictEpoch) + ", dictEpoch+startTime = " + String(dictEpoch+startTime)+ ", dictEpoch+endTime = " + String(dictEpoch+endTime));
    if((nowEpoch > dictEpoch-24*60*60+startTime) && (nowEpoch < dictEpoch-24*60*60+endTime)){
      if(dictEpoch != triggerEpoch){
        acknowledge = 0; //acknowledge only valid for same triggerEpoch
        Serial.println("Resetting since new trigger!");
      } 
      triggerEpoch = dictEpoch;
      if(!acknowledge){
        taskId = entry.second;
      }
    }    
  }
  setColor(taskId);
  FastLED.show();
}

void handleReed(){
  reed.update();  
  initialized = true;
}

void setAcknowledge(){
  unsigned long now = millis();
  if(initialized){
    Serial.println("OFF - Acknowedge!");
    acknowledge = 1;
    //Multi-click detection
    switchCounter++;
    if(((now-lastSwitchMillis)>multiClickTimeout) || (lastSwitchMillis == 0)){switchCounter=0;}
    Serial.println("Counter = " + String(switchCounter));
    lastSwitchMillis = now;
    if(switchCounter == 2){
      if(STATE == STATE_CONFIGURE){
        STATE = STATE_INIT; //reset
      } else {
        STATE = STATE_CONFIGURE;
      }
    }
  }  
}

void doNothing(){
  Serial.println("ON");
  true;  
}

void setBrightness(int taskId){
  if(taskId != taskIdLast){
      brightness = 0;
      taskIdLast = taskId;
  }
  leds[0].fadeLightBy(brightness);
  brightness = brightness + fadeAmount;
  if(brightness<0){brightness = 0;}
  if(brightness>255){brightness = 255;}    
//    Serial.println("Brightness = " + String(brightness) + ", ColorIndex = " + String(taskId));
  if(brightness <= 0 || brightness >= 255){fadeAmount = -fadeAmount;} 
  delay(20);   
}

void setColor(int taskId){
  if(taskId != -1){
    // EVENT
//    Serial.println("Event: " + task[taskId] + " => LED = " + String(color[taskId],HEX));
    leds[0] = color[taskId];
    setBrightness(taskId);  
  } else {
//    Serial.println("No Event");
    leds[0] = CRGB::Black;
  }
}

/// DEBUG
int taskId=0;
int intervall = 8000;
void testLeds(){
  if((millis()-millisLast) > intervall){
    taskId++;
    if(taskId>3) taskId=0;
    millisLast = millis();
  }
  setColor(taskId);
  FastLED.show();
}

/// Splash Screen
void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}