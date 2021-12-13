#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <WiFiUdp.h>

#define MAX_DEVICES 4 //number of led matrix connect in series
#define CS_PIN 15
#define CLK_PIN 14
#define DATA_PIN 12

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

char* ssid = "***REMOVED***";
char* password = "***REMOVED***";

unsigned long interval = 86400000;
unsigned long nextUpdateTime = 0;
String yesterday = "";

// SOFTWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

const int httpsPort = 443;
const String url = "https://api.coingecko.com/api/v3/simple/price?ids=ethereum&vs_currencies=USD";
const String historyURL = "https://api.coingecko.com/api/v3/coins/ethereum/history?date=";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

struct Data {
  String price;
  String yesterdayChange;
};

void setup() {
  Serial.begin(115200);
  P.begin();
  connectingWifi();
  timeClient.begin();
}

void connectingWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    displayMsg("Connecting...", 1000);
  }
  displayMsg("Connected!", 5000);
}

void loop() {
  String date = getCurrentDate();
  Data d = getData(date);
  String text = d.price + " ETH/USD " + "   " + d.yesterdayChange + "%";
  const char* msg = {text.c_str()};
  displayMsg(msg, 300000);
}

String getCurrentDate() {
  unsigned long currentTime = millis();
  if (nextUpdateTime - currentTime < 0 || nextUpdateTime - currentTime > interval) {
    timeClient.update();
    long todayS = timeClient.getEpochTime();
    
    unsigned long timeTillUpdateMs = (interval - (todayS % interval / 1000)) * 1000;
    nextUpdateTime = millis() + timeTillUpdateMs;
    
    unsigned long yesterdayS = todayS - interval / 1000;
    char buffer[80];
    sprintf(buffer, "%02d-%02d-%04d", day(yesterdayS), month(yesterdayS), year(yesterdayS));
    yesterday = String(buffer);
  }
  return yesterday;
}

Data getData(String date) {

  HTTPClient http;

  http.begin(url);
  int httpCode = http.GET();
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, http.getString());

  if (error) {
    return getErrorIfOccur(error);
  }

  double price = doc["ethereum"]["usd"].as<String>().toDouble();
  http.end();

  http.begin(historyURL + date);
  int historyHttpCode = http.GET();
  DynamicJsonDocument historyDoc(16000);
  DeserializationError historyError = deserializeJson(historyDoc, http.getString());

  if (historyError) {
    return getErrorIfOccur(historyError);
  }

  double yesterdayPrice = historyDoc["market_data"]["current_price"]["usd"].as<String>().toDouble();
  http.end();

  double percentChange = ((price - yesterdayPrice) / yesterdayPrice) * 100;

  return (Data) {
    String(price, 0), String(percentChange, 1)
  };
}

Data getErrorIfOccur(DeserializationError error) {
  if (error) {
    Serial.print(F("deserializeJson Failed"));
    Serial.println(error.f_str());
    delay(2500);
    return (Data) {
      "failed", "deserilize data"
    };
  }
}

void displayMsg(const char* text, long milis) {
  long startTime = millis();
  long elapsedTime = 0;
  while (elapsedTime < milis) {
    if (P.displayAnimate())
    {
      P.displayText(text, PA_LEFT, 48, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    }
    elapsedTime = millis() - startTime;
  }
}
