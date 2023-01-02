//Why data is send 10 times???

#include <ESP8266WebServer.h>
ESP8266WebServer server(80);

// Server Functions

void readSettings() { //transfering ESP data to the Webpage
  String value = readFile("/data.json");
  Serial.println("Received:" + value);
  Serial.println("Sending settings: " + value);
  server.send(200, "text/plane", value);
}

void sendSettings() { //transfering start/endHour to the Webpage
  String value;
  value = String(startHour) + "," + String(endHour);
  Serial.println("Sending settings: " + value);
  server.send(200, "text/plane", value);
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

void receiveSettings() {
  String jsonText = server.arg("value");
  String response = "";
  Serial.println("Receiving settings in JSON format: " + jsonText);
  if (writeFile("/data.json", jsonText.c_str())) {
    response = "Übertagen der Daten war erfolgreich!";
  } else {
    response = "ERROR: Übertagen der Daten fehlgeschlagen!";
  }
  server.send(200, "text/plane", response);
}

void setEndHour() {
  String hour = server.arg("value");
  Serial.println("Setting endHour = " + hour);
  endHour = hour.toInt();
  setColor(CRGB::White, false);
  setColor(CRGB::Purple, false);
}

void closeSettings() {
  Serial.println("Closing Settings.");
  STATE_NEXT = STATE_INIT;
}

void deleteSettings() {
  Serial.println("Delete Settings.");
  deleteFile("/data.json");
  ;
}

void startWebServer() {
  Serial.println("Starting WebServer...");
  server.on("/", handleRoot);
  server.on("/set_start", setStartHour);
  server.on("/set_end", setEndHour);
  server.on("/request_settings", sendSettings);
  server.on("/send_settings", receiveSettings);  //webpage => ESP name
  server.on("/read_settings", readSettings);
  server.on("/delete_settings", deleteSettings);
  server.on("/close", closeSettings);
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
