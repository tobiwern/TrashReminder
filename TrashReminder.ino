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
{1671494400,1}, //dummy
};
const String task[] = {"Altpapier-Tonne in Hirrlingen", "Gelber Sack in Hirrlingen", "Häckselgut in Hirrlingen", "Restmüll in Hirrlingen"};
//         yellow, green, white
const int color[] = {0x0000FF, 0xFFFF00, 0x00FF00, 0xFFFFFF };
const int validIndex[] = {0,1,2,3};
int startTime = 15*60*60; //seconds => 15 Uhr
int endTime = (24+8)*60*60; //seconds => nächster Tag 8 Uhr morgens

#include <FastLED.h>
#define NUM_LEDS 1
#define DATA_PIN 4 //D2
CRGB leds[NUM_LEDS];

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
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
  timeClient.update();
  int nowEpoch = timeClient.getEpochTime();
//  handleLed(nowEpoch);
  testLeds();
}

void handleLed(int nowEpoch){
  int dictEpoch;
  int taskId = -1;
  for (auto entry :epochTaskDict)  {
    dictEpoch = entry.first;
//    Serial.println("now: " + String(nowEpoch) + ", dictEpoch = " + String(dictEpoch) + ", dictEpoch+startTime = " + String(dictEpoch+startTime)+ ", dictEpoch+endTime = " + String(dictEpoch+endTime));
    if((nowEpoch > dictEpoch+startTime) && (nowEpoch < dictEpoch+endTime)){
      taskId = entry.second;
      if()
    }    
  }
  if(taskId != -1){
    Serial.println("Event: " + task[taskId] + " => LED = " + String(color[taskId]));
    leds[0] = color[taskId];
  } else {
    Serial.println("No Event");
    leds[0] = CRGB::Black;
  }
  FastLED.show();
  delay(60000); // 1 minute
}

int taskId=0;
void testLeds(){
  taskId++;
  if(taskId>3) taskId=0;
  leds[0] = color[taskId]; 
  FastLED.show();
  delay(5000);
}