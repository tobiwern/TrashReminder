//https://arduinojson.org/v6/api/jsondocument/
//https://arduinojson.org/v6/doc/deserialization/
//https://arduinojson.org/v6/assistant/#/step1
//https://arduinogetstarted.com/reference/serial-readbytes
//https://makesmart.net/arduino-ide-arbeiten-mit-json-objekten-fur-einsteiger/
//https://arduinojson.org/v6/how-to/reduce-memory-usage/
//https://github.com/arkhipenko/Dictionary

#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#define JSON_MEMORY 1024 * 30
// Server Functions

void printTaskIds(int taskIds[]) {  //debug
  Serial.print("taskIds[] = ");
  for (int i = 0; i < maxNumberOfTasksPerDay; i++) {
    if (taskIds[i] != -1) { Serial.print(String(taskIds[i]) + " "); }
  }
  Serial.println("");
}

boolean initDataFromFile() {
  if (!startLittleFS()) { return (""); }
  Serial.printf("INFO: Reading file: %s\n", dataFile);
  File file = LittleFS.open(dataFile, "r");
  if (!file) {
    Serial.println("INFO: Failed to open file " + String(dataFile) + " for reading!");
    return (false);
  }
  DynamicJsonDocument doc(JSON_MEMORY);  //on heap for large amount of data
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return (false);
  }
  // get validTasks ////////////////////////////////
  JsonArray validTaskIds = doc["validTaskIds"];  //Implicit cast
  numberOfValidTaskIds = 0;
  for (JsonVariant v : validTaskIds) {
    validTaskId[numberOfValidTaskIds++] = v.as<int>();
    Serial.println("validTaskId: " + String(validTaskId[numberOfValidTaskIds - 1]));
  }
  // get tasks ////////////////////////////////
  JsonArray tasks = doc["tasks"];  //Implicit cast
  numberOfTaskIds = 0;
  for (String taskText : tasks) {  //Implicit cast
    task[numberOfTaskIds++] = taskText;
    Serial.println("Task: " + task[numberOfTaskIds - 1]);
  }
  // get colors ////////////////////////////////
  JsonArray colors = doc["colors"];  //implicit cast to JsonArray!
  numberOfTaskIds = 0;
  for (String colorText : colors) {
    unsigned long int color1 = strtoul(colorText.c_str(), NULL, 16);  //conversion from HEX String => HEX number
    color[numberOfTaskIds++] = color1;
    Serial.println("Color: " + colorText + ", value = " + String(color1));
  }

  JsonArray epochTasks = doc["epochTasks"];  //Implicit cast
  numberOfEpochs = 0;
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
          Serial.println("WARNING: Too many taskIds: SKIPPING: " + taskId);
        }
      }
      //JsonArray taskIdArray = p.value();                      // is a JsonVariant
      copyArray(taskIdArray, taskIds);
      epochTask entry;
      entry.epoch = epoch;
      memcpy(entry.taskIds, taskIds, sizeof(entry.taskIds));
      if (numberOfEpochs < maxNumberOfEpochs - 1) {  //prevent running over reserved memory
        epochTaskDict[numberOfEpochs++] = entry;     //{ .epoch = epoch, .taskIds = taskIds };
      } else {
        Serial.println("WARNING: Too many entries: SKIPPING: ");
      }
      Serial.print("epoch: " + String(epoch) + ", ");
      printTaskIds(taskIds);
    }
  }
  file.close();
  endLittleFS();
  return (true);
}

//settings
void initStartEndTimes() {
  String jsonText = readFile(settingsFile);
  if (jsonText == "") {
    Serial.println("ERROR: Lesen der Daten fehlgeschlagen.");
    return;
  }
  Serial.println("Read settings: " + jsonText);
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, jsonText);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  startHour = doc["startHour"];
  endHour = doc["endHour"];
  //  Serial.println("Read startHour = " + String(startHour) + ", endHour = " + String(endHour));
}

void sendTasksToWebpage() {  //transfering ESP data to the Webpage
  String value = readFile(dataFile);
  if (value != "") {
    Serial.println("Received:" + value);
    Serial.println("Sending taks: " + value);
    server.send(200, "text/plane", value);
  } else {
    value = "ERROR";
    server.send(500, "text/plane", value);
  }
}

void sendSettingsToWebpage() {  //transferring ESP settings => Webpage
  String value;
  value = String(startHour) + "," + String(endHour) + "," + maxNumberOfEpochs + "," + maxNumberOfTasksPerDay + "," + maxNumberOfTaskIds;
  Serial.println("Sending settings: " + value);
  server.send(200, "text/plane", value);
}

void notFound() {
  Serial.println("Not found");
  server.send(404, "text/plain", "Not found");
}

void handleRoot() {
  String s = webpage;
  server.send(200, "text/html", s);
}

void writeStartEndTimes() {
  String jsonText = "";
  jsonText = "{\"startHour\":" + String(startHour) + ",\"endHour\":" + String(endHour) + "}";
  Serial.println("Writing settings: " + jsonText);
  writeFile("/settings.json", jsonText.c_str());
}

void setStartHour() {
  String hour = server.arg("value");
  Serial.println("Setting startHour = " + hour);
  startHour = hour.toInt();
  setColor(CRGB::White, false);
  setColor(CRGB::Purple, false);
  server.send(200, "text/plane", "Neuer Startzeitpunkt übertragen.");
  writeStartEndTimes();
}

void setEndHour() {
  String hour = server.arg("value");
  Serial.println("Setting endHour = " + hour);
  endHour = hour.toInt();
  setColor(CRGB::White, false);
  setColor(CRGB::Purple, false);
  server.send(200, "text/plane", "Neuer Endzeitpunkt übertragen.");
  writeStartEndTimes();
}

void receiveFromWebpage_Tasks() {
  String jsonText = server.arg("value");
  Serial.println("Receiving settings in JSON format: " + jsonText);
  if (writeFile(dataFile, jsonText.c_str())) {
    server.send(200, "text/plane", "OK");
  } else {
    server.send(500, "text/plane", "ERROR");
  }
}

boolean receiveFromWebpage_ValidTaskIds() {
  String jsonText = server.arg("value");
  Serial.println("Receiving settings in JSON format: " + jsonText);
  //First translate validTaskIds into JsonArray
  StaticJsonDocument<128> doc1;
  DeserializationError error1 = deserializeJson(doc1, jsonText);
  if (error1) {
    Serial.print(F("A: deserializeJson() failed: "));
    Serial.println(error1.f_str());
    server.send(500, "text/plane", "ERROR");
    return (false);
  }
  JsonArray validTaskIds = doc1["validTaskIds"];

  //Open old document
  if (!startLittleFS()) { return (false); }
  Serial.printf("INFO: Reading file: %s\n", dataFile);
  File file = LittleFS.open(dataFile, "r");  
  if (!file) {
    Serial.println("INFO: Failed to open file " + String(dataFile) + " for reading!");
    server.send(500, "text/plane", "ERROR");
    return (false);
  }
  DynamicJsonDocument doc(JSON_MEMORY);  //on heap for large amount of data
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.print(F("B: deserializeJson() failed: "));
    Serial.println(error.f_str());
    server.send(500, "text/plane", "ERROR");
    return (false);
  }
  doc["validTaskIds"] = validTaskIds; //update with new data
  file.close();
  
  file = LittleFS.open(dataFile, "w");  
  if (!file) {
    Serial.println("INFO: Failed to open file " + String(dataFile) + " for writing!");
    server.send(500, "text/plane", "ERROR");
    return (false);
  }

  serializeJson(doc, file); //save back to data.json
  file.close();
  endLittleFS();

  server.send(200, "text/plane", "OK");
  return (true);
}

void closeSettings() {
  Serial.println("Closing Settings.");
  STATE_NEXT = STATE_INIT;
}

void fireworks() {
  Serial.println("Fireworks...");
  STATE_FOLLOWING = STATE_CONFIGURE;
  STATE_NEXT = STATE_SHOW;
  millisLast = millis();                 //to reset show timer
  server.send(200, "text/plane", "OK");  //should always respond to prevent resend (10x)
}

void deleteTasks() {
  Serial.println("Delete Settings.");
  if (deleteFile(dataFile)) {
    server.send(200, "text/plane", "OK");
  } else {
    server.send(500, "text/plane", "ERROR");
  }
}

void startWebServer() {
  Serial.println("Starting WebServer...");
  server.on("/", handleRoot);
  server.on("/set_start", setStartHour);
  server.on("/set_end", setEndHour);
  server.on("/request_settings", sendSettingsToWebpage);
  server.on("/request_tasks", sendTasksToWebpage);     //ESP => webpage
  server.on("/send_tasks", receiveFromWebpage_Tasks);  //webpage => ESP name
  server.on("/delete_tasks", deleteTasks);
  server.on("/close", closeSettings);
  server.on("/fireworks", fireworks);
  server.on("/send_ValidTaskIds", receiveFromWebpage_ValidTaskIds);
  server.onNotFound(notFound);
  server.begin();
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print(", hostName: http://");
  Serial.println(WiFi.getHostname());
}

void stopWebServer() {
  Serial.println("Stopping WebServer...");
  server.stop();
}
