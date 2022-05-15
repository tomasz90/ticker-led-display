#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

char* ssid = "***REMOVED***";
char* password = "***REMOVED***";

struct Data {
  String price;
  String yesterdayChange;
};

void setup() {
  Serial.begin(115200);
  beginDisplaying();
  connectWifi();
}

void loop() {
  String rates = getData();
  updateText(rates);
  delay(15000);
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
}

void connectWifi() {
  WiFi.begin(ssid, password);
  updateText("Connecting...");
  restartIfNotConnectedOnTime(15000);
  updateText("Connected!");
}

void restartIfNotConnectedOnTime(long maxDuration) {
  long start = millis();
  long duration = 0;
  while (duration < maxDuration) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("connected!");
      return;
    }
    duration = millis() - start;
    delay(100);
  }
  ESP.restart();
}
