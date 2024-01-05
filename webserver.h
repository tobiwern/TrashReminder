//#include "wiring_private.h" //don't know what this is?
//https://arduinojson.org/v6/api/jsondocument/
//https://arduinojson.org/v6/doc/deserialization/
//https://arduinojson.org/v6/assistant/#/step1
//https://arduinogetstarted.com/reference/serial-readbytes
//https://makesmart.net/arduino-ide-arbeiten-mit-json-objekten-fur-einsteiger/
//https://arduinojson.org/v6/how-to/reduce-memory-usage/
//https://github.com/arkhipenko/Dictionary

#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
boolean serverRunning = false;
#define JSON_MEMORY 1024 * 30

void printTaskIds(int taskIds[]) {  //debug
  DEBUG_SERIAL.print("taskIds[] = ");
  for (int i = 0; i < maxNumberOfTasksPerDay; i++) {
    if (taskIds[i] != -1) { DEBUG_SERIAL.print(String(taskIds[i]) + " "); }
  }
  DEBUG_SERIAL.println("");
}

boolean initDataFromFile() {
  if (!startLittleFS()) { return (""); }
  DEBUG_SERIAL.printf("INFO: Reading file: %s\n", dataFile);
  File file = LittleFS.open(dataFile, "r");
  numberOfValidTaskIds = 0;
  numberOfTaskIds = 0;
  int numberOfColors = 0;  //only locally needed
  numberOfEpochs = 0;
  if (!file) {
    DEBUG_SERIAL.println("INFO: Failed to open file " + String(dataFile) + " for reading!");
    return (false);
  }
  DynamicJsonDocument doc(JSON_MEMORY);  //on heap for large amount of data
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    DEBUG_SERIAL.print("deserializeJson() failed: ");
    DEBUG_SERIAL.println(error.f_str());
    return (false);
  }
  // get validTasks ////////////////////////////////
  JsonArray validTaskIds = doc["validTaskIds"];  //Implicit cast
  for (JsonVariant v : validTaskIds) {
    validTaskId[numberOfValidTaskIds++] = v.as<int>();
    DEBUG_SERIAL.println("validTaskId: " + String(validTaskId[numberOfValidTaskIds - 1]));
  }
  // get tasks ////////////////////////////////
  JsonArray tasks = doc["tasks"];  //Implicit cast
  for (String taskText : tasks) {  //Implicit cast
    task[numberOfTaskIds++] = taskText;
    DEBUG_SERIAL.println("Task: " + task[numberOfTaskIds - 1]);
  }
  // get colors ////////////////////////////////
  JsonArray colors = doc["colors"];  //implicit cast to JsonArray!
  for (String colorText : colors) {
    unsigned long int color1 = strtoul(colorText.c_str(), NULL, 16);  //conversion from HEX String => HEX number
    color[numberOfColors++] = color1;
    DEBUG_SERIAL.println("Color: " + colorText + ", value = " + String(color1));
  }
  JsonArray epochTasks = doc["epochTasks"];  //Implicit cast
  for (JsonObject obj : epochTasks) {
    for (JsonPair p : obj) {
      int taskIds[maxNumberOfTasksPerDay];
      memset(taskIds, -1, sizeof(taskIds));
      unsigned long epoch = strtoul(p.key().c_str(), 0, 10);  // is a JsonString
      int counter = 0;
      JsonArray taskIdArray = p.value();  // is a JsonVariant
      for (int taskId : taskIdArray) {
        if (counter < maxNumberOfTasksPerDay - 1) {  //prevent running over reserved memory
          taskIds[counter++] = taskId;
        } else {
          DEBUG_SERIAL.println("WARNING: Too many taskIds: SKIPPING: " + taskId);
        }
      }
      //JsonArray taskIdArray = p.value();                      // is a JsonVariant
      //copyArray(taskIdArray, taskIds);
      epochTask entry;
      entry.epoch = epoch;
      memcpy(entry.taskIds, taskIds, sizeof(entry.taskIds));
      if (numberOfEpochs < maxNumberOfEpochs - 1) {  //prevent running over reserved memory
        epochTaskDict[numberOfEpochs++] = entry;     //{ .epoch = epoch, .taskIds = taskIds };
      } else {
        DEBUG_SERIAL.println("WARNING: Too many entries: SKIPPING: ");
      }
      DEBUG_SERIAL.print("epoch: " + String(epoch) + ", ");
      printTaskIds(taskIds);
    }
  }
  file.close();
  endLittleFS();
  return (true);
}

//settings
void initStartEndTimes() {
  if (!startLittleFS()) { return; }
  DEBUG_SERIAL.printf("INFO: Reading file: %s\n", settingsFile);
  File file = LittleFS.open(settingsFile, "r");
  if (!file) {
    DEBUG_SERIAL.println("INFO: Failed to open file " + String(settingsFile) + " for reading!");
    return;
  }
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    DEBUG_SERIAL.print("deserializeJson() failed: ");
    DEBUG_SERIAL.println(error.f_str());
    return;
  }
  startHour = doc["startHour"];
  endHour = doc["endHour"];
  file.close();
  endLittleFS();
}

void sendTasksToWebpage() {  //transfering ESP data to the Webpage
  String value = readFile(dataFile);
  if (value != "") {
    DEBUG_SERIAL.println("Received:" + value);
    DEBUG_SERIAL.println("Sending taks: " + value);
    server.send(200, "text/plane", value);
  } else {
    value = "ERROR";
    server.send(500, "text/plane", value);
  }
}

void sendSettingsToWebpage() {  //transferring ESP settings => Webpage
  String value;
  value = String(startHour) + "," + String(endHour) + "," + maxNumberOfEpochs + "," + maxNumberOfTasksPerDay + "," + maxNumberOfTaskIds;
  DEBUG_SERIAL.println("Sending settings: " + value);
  server.send(200, "text/plane", value);
}

void notFound() {
  DEBUG_SERIAL.println("Not found");
  server.send(404, "text/plain", "Not found");
}

void handleRoot() {
  String s = webpage;
  server.send(200, "text/html", s);
}

void writeStartEndTimes() {
  String jsonText = "";
  jsonText = "{\"startHour\":" + String(startHour) + ",\"endHour\":" + String(endHour) + "}";
  DEBUG_SERIAL.println("Writing settings: " + jsonText);
  writeFile("/settings.json", jsonText.c_str());
  initStartEndTimes();
}

void setStartHour() {
  String hour = server.arg("value");
  DEBUG_SERIAL.println("Setting startHour = " + hour);
  startHour = hour.toInt();
  setColor(CRGB::White, false);
  setColor(CRGB::Purple, false);
  server.send(200, "text/plane", "Neuer Startzeitpunkt gespeichert.");
  writeStartEndTimes();
}

void setEndHour() {
  String hour = server.arg("value");
  DEBUG_SERIAL.println("Setting endHour = " + hour);
  endHour = hour.toInt();
  setColor(CRGB::White, false);
  setColor(CRGB::Purple, false);
  server.send(200, "text/plane", "Neuer Endzeitpunkt gespeichert.");
  writeStartEndTimes();
}

void receiveFromWebpage_Tasks() {
  String jsonText = server.arg("value");
  DEBUG_SERIAL.println("Receiving settings in JSON format: " + jsonText);
  if (writeFile(dataFile, jsonText.c_str())) {
    server.send(200, "text/plane", "OK");
    initDataFromFile();
  } else {
    server.send(500, "text/plane", "ERROR");
  }
}

void closeSettings() {
  DEBUG_SERIAL.println("Closing Settings.");
  STATE_NEXT = STATE_INIT;
}

void fireworks() {
  DEBUG_SERIAL.println("Fireworks...");
  STATE_FOLLOWING = STATE_QUERY;
  STATE_NEXT = STATE_SHOW;
  millisLast = millis();                 //to reset show timer
  server.send(200, "text/plane", "OK");  //should always respond to prevent resend (10x)
}

void demo() {
  DEBUG_SERIAL.println("Demo...");
  STATE_NEXT = STATE_DEMO;
  millisLast = millis();                 //to reset show timer
  server.send(200, "text/plane", "OK");  //should always respond to prevent resend (10x)
}

void deleteTasks() {
  DEBUG_SERIAL.println("Delete Settings.");
  if (deleteFile(dataFile)) {
    server.send(200, "text/plane", "OK");
    initDataFromFile();
  } else {
    server.send(500, "text/plane", "ERROR");
  }
  //STATE_NEXT = STATE_INIT;
}

void resetWifiSettings() {
  DEBUG_SERIAL.println("Reset Wifi Settings.");
  WiFiManager wm;  //Is this fine to have another instance?
  wm.resetSettings();
  server.send(200, "text/plane", "OK");
  //  leds[0] = CRGB::Red;                                      //in case no successful WiFi connection
  //  FastLED.show();
  ESP.restart();
}

void receiveFromWebpage_Acknowledge() {  //ToDo: Need to version webpages
  DEBUG_SERIAL.println("Acknowledge...");
  acknowledge = 1;
  server.send(200, "text/plane", "OK");  //should always respond to prevent resend (10x)
}

void startWebServer() {
  DEBUG_SERIAL.println("Starting WebServer...");
  server.on("/", handleRoot);
  server.on("/set_start", setStartHour);
  server.on("/set_end", setEndHour);
  server.on("/request_settings", sendSettingsToWebpage);
  server.on("/request_tasks", sendTasksToWebpage);     //ESP => webpage
  server.on("/send_tasks", receiveFromWebpage_Tasks);  //webpage => ESP name
  server.on("/delete_tasks", deleteTasks);
  server.on("/close", closeSettings);
  server.on("/fireworks", fireworks);
  server.on("/demo", demo);
  server.on("/acknowledge", receiveFromWebpage_Acknowledge);
  server.on("/reset_wifi_settings", resetWifiSettings);
  //  server.on("/send_ValidTaskIds", receiveFromWebpage_ValidTaskIds);
  server.onNotFound(notFound);
  server.begin();
  DEBUG_SERIAL.print("IP address: ");
  DEBUG_SERIAL.print(WiFi.localIP());
  DEBUG_SERIAL.print(", hostName: http://");
  DEBUG_SERIAL.println(WiFi.getHostname());
  serverRunning = true;
}

void stopWebServer() {
  DEBUG_SERIAL.println("Stopping WebServer...");
  server.stop();
  serverRunning = false;
}

boolean isClientConnected() {
  WiFiClient myclient = server.client();
  return (myclient.connected());
}
