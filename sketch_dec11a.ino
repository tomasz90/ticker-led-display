#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <WiFiUdp.h>

#define MAX_DEVICES 10 //number of led matrix connect in series
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
const String coinUrl = "https://api.binance.com/api/v3/ticker/price?symbol=COINUSDT";
const String coinHistoryUrl = "https://api.binance.com/api/v3/ticker/24hr?symbol=COINUSDT";

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
  Data btc = getData("BTC");
  Data eth = getData("ETH");
  String wholeMsg = eth.price + " ETH/USD " + "  " + eth.yesterdayChange + "%" + "        " +  btc.price + " BTC/USD " + "  " + btc.yesterdayChange + "%";
  const char* msg = (wholeMsg).c_str();
  P.setIntensity(15);
  displayText(msg);
  delay(300000);
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
}

void connectWifi() {
  WiFi.begin(ssid, password);
  displayText("Connecting...");
  delay(20000);
  if (WiFi.status() != WL_CONNECTED) {
    ESP.restart();
  }
  displayText("Connected!");
}

Data getData(String coin) {

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

  double price = doc["price"].as<String>().toDouble();
  http.end();

  String historyUrl = coinHistoryUrl;
  historyUrl.replace("COIN", coin);
  
  http.begin(historyUrl);
  int historyHttpCode = http.GET();
  DynamicJsonDocument historyDoc(8000);
  DeserializationError historyError = deserializeJson(historyDoc, http.getString());

  if (historyError) {
    return getErrorIfOccur(historyError);
  }

  double percentChange = historyDoc["priceChangePercent"].as<String>().toDouble();
  http.end();

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
    5000,
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
