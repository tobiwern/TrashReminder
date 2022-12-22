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

std::unordered_map<int,String>epochTaskDict = {
{1685750400,"4"},
{1686873600,"4,1"},
{1695600000,"0"},
{1686355200,"1"},
{1676592000,"1"},
{1682035200,"4"},
{1694736000,"1"},
{1685923200,"0"},
{1699833600,"2"},
{1692316800,"1"},
{1701043200,"2"},
{1692921600,"4,1"},
{1678406400,"4"},
{1688688000,"1"},
{1703894400,"4"},
{1683849600,"1"},
{1681171200,"0"},
{1673827200,"0"},
{1676851200,"2"},
{1674777600,"4"},
{1699574400,"1"},
{1682985600,"2"},
{1695340800,"4"},
{1690761600,"0"},
{1690156800,"2"},
{1679270400,"2"},
{1687737600,"2"},
{1673222400,"2"},
{1683504000,"0"},
{1701993600,"1"},
{1688947200,"2"},
{1674432000,"2"},
{1693180800,"0"},
{1702252800,"2"},
{1673568000,"4"},
{1675641600,"2"},
{1689897600,"1"},
{1684540800,"4"},
{1678060800,"2"},
{1688342400,"0"},
{1698624000,"2"},
{1695945600,"1"},
{1687478400,"1"},
{1674172800,"1"},
{1681689600,"2"},
{1694131200,"4,1"},
{1679616000,"4"},
{1688083200,"4,1"},
{1698364800,"1"},
{1685059200,"1"},
{1691107200,"1"},
{1689292800,"4,1"},
{1699056000,"4"},
{1691712000,"4,1"},
{1697155200,"1"},
{1701388800,"4"},
{1703203200,"1"},
{1680912000,"4"},
{1677196800,"4"},
{1702857600,"0"},
{1683331200,"4"},
{1682640000,"1"},
{1690502400,"4,1"},
{1702598400,"4"},
{1681516800,"1"},
{1673049600,"1"},
{1679011200,"1"},
{1680480000,"2"},
{1676246400,"0"},
{1698969600,"3"},
{1697414400,"2"},
{1693785600,"2"},
{1703289600,"2"},
{1700179200,"4"},
{1700784000,"1"},
{1675987200,"4"},
{1696636800,"4"},
{1697760000,"4"},
{1680134400,"3"},
{1675382400,"1"},
{1692576000,"2"},
{1698019200,"0"},
{1694995200,"2"},
{1693526400,"1"},
{1680220800,"1"},
{1700438400,"0"},
{1696204800,"2"},
{1678665600,"0"},
{1686528000,"2"},
{1684108800,"2"},
{1677801600,"1"},
{1685404800,"2"},
{1691366400,"2"},
{1672099200,"1"}, //2022.12.27
{1672444800,"3"}, //2022.12.31
{1671753600,"0,2,3"}, //dummy
{1671840000,"2,1"}, //dummy 2022.12.24 show
};
//                      0                                1                          2                            3                           4
const String task[] = {"Altpapier-Tonne in Hirrlingen", "Bioabfall in Hirrlingen", "Gelber Sack in Hirrlingen", "Häckselgut in Hirrlingen", "Restmüll in Hirrlingen"};
const int validIndex[] = {0,2,3,4};
//                   blue,     brown,    yellow,   green,    white
const int color[] = {0x0000FF, 0xA52A2A, 0xFFFF00, 0x00FF00, 0xFFFFFF };

int colorIds[] = {-1,-1,-1}; //allow max three tasks on the same day
int colorIdsLast[] = {-1,-1,-1};
int numberOfColorIds = sizeof(colorIds)/sizeof(colorIds[0]);
int colorIndex = 0; //used to toggle between multiple colors if required

int startTime = 11*60*60; //seconds => 15 Uhr
int endTime = (24+8)*60*60; //seconds => nächster Tag 8 Uhr morgens
int taskIdLast = -1;
int brightness = 255;
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
#define BRIGHTNESS         0
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
  memset(colorIds, -1, sizeof(colorIds));
//  String taskId = "-1";
  for (auto entry :epochTaskDict)  {
    dictEpoch = entry.first;
//    Serial.println("now: " + String(nowEpoch) + ", dictEpoch = " + String(dictEpoch) + ", dictEpoch+startTime = " + String(dictEpoch+startTime)+ ", dictEpoch+endTime = " + String(dictEpoch+endTime));
    if((nowEpoch > dictEpoch-24*60*60+startTime) && (nowEpoch < dictEpoch-24*60*60+endTime)){
      if(dictEpoch != triggerEpoch){
        acknowledge = 0; //acknowledge only valid for same triggerEpoch
        colorIndex = 0;
        Serial.println("Resetting since new trigger!");
      } 
      triggerEpoch = dictEpoch;
      if(!acknowledge){
        setColorIds(entry.second);
      }
    }    
  }
  setTaskColor();
  FastLED.show();
}

boolean isValidTask(int taskId){
  for(int i = 0; i < sizeof(validIndex) / sizeof(validIndex[0]); i++){
    if(taskId == validIndex[i]){return(true);}
  }
  return(false);
}

void setColorIds(String taskIds){
  int start = 0, index = 0, indexFound, taskId; 
  do{
    indexFound = taskIds.indexOf(",", start); 
    taskId = taskIds.substring(start, indexFound).toInt();
    if(isValidTask(taskId)){colorIds[index++] = taskId;}
//    Serial.println("taskId = " + String(taskId) + ", start = " + String(start) + ", indexFound = " + String(indexFound) + ", validTask = " + String(isValidTask(taskId)));
    start=indexFound+1;    
  } while (indexFound != -1);
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

void incrementColorId(){
  if(colorIds[1]!=-1){
    colorIndex++;
//    Serial.println("numberOfColorIds = " + String(numberOfColorIds));
    if(colorIndex >= numberOfColorIds){colorIndex = 0;}
  }
}

void setBrightness(boolean reset = false){
  if(reset){
    brightness = 255;
  } else {
    if((colorIds[0] != colorIdsLast[0]) || (colorIds[1] != colorIdsLast[1])){brightness = 255;}
    colorIdsLast[0] = colorIds[0];
    colorIdsLast[1] = colorIds[1];
  }
  leds[0].fadeLightBy(brightness);
  brightness = brightness + fadeAmount;
  if(brightness<=0){brightness = 0; }
  if(brightness>=255){brightness = 255; incrementColorId();}    
//    Serial.println("Brightness = " + String(brightness) + ", ColorIndex = " + String(taskId));
  if(brightness <= 0 || brightness >= 255){fadeAmount = -fadeAmount;} 
  delay(20);   
}

void setTaskColor(){
  if(colorIds[0] == -1){
//    Serial.println("No Event");
    leds[0] = CRGB::Black;
  } else {
    // EVENT
//    Serial.println("Event: " + task[taskId] + " => LED = " + String(color[taskId],HEX));
//        for (int i=0; i<sizeof arr/sizeof arr[0]; i++) {
//        int s = arr[i];    
    leds[0] = color[colorIds[colorIndex]];
    setBrightness();  
  }
}

void setColor(int color, boolean fade = true){
  leds[0] = color;
  if(fade){setBrightness();}
  FastLED.show();
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
  setTaskColor();
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
  addGlitter(50);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void handleState(){
  unsigned long millisNow = millis();
  if(STATE != STATE_PREVIOUS) {Serial.println("STATE = " + stateTbl[STATE]);}
  switch (STATE) {
    case STATE_INIT:                //***********************************************************
      millisLast = millisNow;
      brightness = 255;
      colorIndex = 0;
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
      setColor(CRGB::Red);
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
      setColor(CRGB::Purple, false);
      handleReed();
      break;       
    default:
      break;
  }   
  STATE_PREVIOUS = STATE; 
}