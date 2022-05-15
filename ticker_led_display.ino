#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define MAX_DEVICES 10 //number of led matrix connect in series
#define CS_PIN 15
#define CLK_PIN 14
#define DATA_PIN 12

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

char* ssid = "***REMOVED***";
char* password = "***REMOVED***";

TaskHandle_t taskHandle = NULL;

// SOFTWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

struct Data {
  String price;
  String yesterdayChange;
};

void setup() {
  Serial.begin(115200);
  P.begin();
  P.setIntensity(15);
  P.setScrollSpacing(9);
  connectWifi();
}

void loop() {
  Serial.println("Http requests begin.");
  Data btc = getData("BTC");
  Data eth = getData("ETH");
  String wholeMsg = eth.price + " ETH/USD " + "  " + eth.yesterdayChange + "%     " +  btc.price + " BTC/USD " + "  " + btc.yesterdayChange + "%     ";
  const char* msg = (wholeMsg).c_str();
  Serial.println("Refreshing whith new rates.");
  displayText(msg);
  delay(600000);
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
}

void connectWifi() {
  WiFi.begin(ssid, password);
  displayText("Connecting...");
  restartIfNotConnectedOnTime(15000);
  displayText("Connected!");
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
