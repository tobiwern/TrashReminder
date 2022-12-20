#include <unordered_map>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <credentials.h>

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
};
task[] = {"Altpapier-Tonne in Hirrlingen", "Gelber Sack in Hirrlingen", "Häckselgut in Hirrlingen", "Restmüll in Hirrlingen"};
//         yellow, green, white
String color[] = {"#0000FF", "#FFFF00", "#00FF00", "#FFFFFF" };

/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/


// Replace with your network credentials
const char *ssid     = mySSID;
const char *password = myPASSWORD;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(3600);
}

void loop() {
  timeClient.update();

  time_t epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);  

  int currentHour = timeClient.getHours();
  Serial.print("Hour: ");
  Serial.println(currentHour);  

  int currentMinute = timeClient.getMinutes();
  Serial.print("Minutes: ");
  Serial.println(currentMinute); 
   
  int currentSecond = timeClient.getSeconds();
  Serial.print("Seconds: ");
  Serial.println(currentSecond);  

  String weekDay = weekDays[timeClient.getDay()];
  Serial.print("Week Day: ");
  Serial.println(weekDay);    

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(monthDay);

  int currentMonth = ptm->tm_mon+1;
  Serial.print("Month: ");
  Serial.println(currentMonth);

  String currentMonthName = months[currentMonth-1];
  Serial.print("Month name: ");
  Serial.println(currentMonthName);

  int currentYear = ptm->tm_year+1900;
  Serial.print("Year: ");
  Serial.println(currentYear);

  //Print complete date:
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  Serial.print("Current date: ");
  Serial.println(currentDate);

  Serial.println("");

  delay(2000);
}
