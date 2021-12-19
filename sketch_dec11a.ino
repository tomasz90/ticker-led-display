#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <WiFiUdp.h>

#define MAX_DEVICES 6 //number of led matrix connect in series
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
const String coinUrl = "https://api.coingecko.com/api/v3/simple/price?ids=COIN&vs_currencies=USD";
const String coinHistoryUrl = "https://api.coingecko.com/api/v3/coins/COIN/history?date=";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

TaskHandle_t taskHandle = NULL;

struct Data {
  String price;
  String yesterdayChange;
};

void setup() {
  Serial.begin(115200);
  P.begin();
  connectWifi();
  timeClient.begin();
}

void loop() {
  String date = getCurrentDate();
  Data btc = getData(date, "bitcoin");
  Data eth = getData(date, "ethereum");
  String btcMsg = btc.price + " BTC/USD " + "  " + btc.yesterdayChange + "%" + "        " + eth.price + " ETH/USD " + "  " + eth.yesterdayChange + "%";
  //String ethMsg = 
  const char* msg = (btcMsg).c_str();
  displayText(msg);
  delay(300000);
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
}

void connectWifi() {
  WiFi.begin(ssid, password);
  displayText("Connecting...");
  delay(10000);
  if (WiFi.status() != WL_CONNECTED) {
    ESP.restart();
  }
  displayText("Connected!");
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

Data getData(String date, String coin) {

  HTTPClient http;

  String url = coinUrl;
  url.replace("COIN", coin);
  
  http.begin(url);
  int httpCode = http.GET();
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, http.getString());

  if (error) {
    return getErrorIfOccur(error);
  }

  double price = doc[coin]["usd"].as<String>().toDouble();
  http.end();

  String historyUrl = coinHistoryUrl;
  historyUrl.replace("COIN", coin);
  
  http.begin(historyUrl + date);
  int historyHttpCode = http.GET();
  DynamicJsonDocument historyDoc(16000);
  DeserializationError historyError = deserializeJson(historyDoc, http.getString());

  if (historyError) {
    return getErrorIfOccur(historyError);
  }

  double yesterdayPrice = historyDoc["market_data"]["current_price"]["usd"].as<String>().toDouble();
  http.end();

  double percentChange = ((price - yesterdayPrice) / yesterdayPrice) * 100;

  String percentChangeStr = "";

  if (percentChange > 0) {
    percentChangeStr = "+";
  }
  percentChangeStr.concat(String(percentChange, 1));

  return (Data) {
    String(price, 0), percentChangeStr
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

void displayText(const void *text) {

  if (taskHandle != NULL) {
    cancelText(taskHandle);
  }

  taskHandle = TaskHandle_t();

  Serial.println("Creating task");
  xTaskCreate(
    animateText,
    "Task 1",
    1000,
    const_cast<void*>(text),
    1,
    &taskHandle
  );
}

void animateText(void *param)
{
  P.displayText((char*)param, PA_LEFT, 48, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  for (;;) {
    if (P.displayAnimate()) // If finished displaying message
    {
      Serial.println("this is still running");
      P.displayReset();
    }
    vTaskDelay(5);
  }
}

void cancelText(TaskHandle_t task1Handle) {
  bool stopped = false;
  while (!stopped) {
    if (P.displayAnimate()) {
      Serial.println("trying delete task..");
      vTaskDelete(task1Handle);
      stopped = true;
    }
  }
}
