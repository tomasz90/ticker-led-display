#include <WiFi.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

struct Data {
  String price;
  String yesterdayChange;
};

WiFiManager manager;

void setup() {
  Serial.begin(115200);
  beginDisplaying();
  updateText("Connecting...");
  manager.setConnectTimeout(10);
  manager.setConfigPortalTimeout(120);
  manager.autoConnect("ESP_AP", "password");
}

void loop() {

  String rates = getData();
  updateText(rates);

  if (WiFi.status() != WL_CONNECTED) {
    if (!WiFi.reconnect()) {
      updateText("Failed connect wifi...");
    }
  }

  delay(30000);
}
