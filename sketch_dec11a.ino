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

//char* ssid = "htspt";
//char* password = "d633d0ec4edd";

TaskHandle_t taskHandle = NULL;

// SOFTWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

const String coinUrl = "https://api.binance.com/api/v3/ticker/24hr?symbol=COINUSDT";

struct Data {
  String price;
  String yesterdayChange;
};

void setup() {
  Serial.begin(115200);
  P.begin();
  P.setIntensity(15);
  connectWifi();
}

void loop() {
  Serial.println("Http requests begin.");
  Data btc = getData("BTC");
  Data eth = getData("ETH");
  String wholeMsg = eth.price + " ETH/USD " + "  " + eth.yesterdayChange + "%" + "        " +  btc.price + " BTC/USD " + "  " + btc.yesterdayChange + "%";
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

Data getData(String coin) {

  String url = coinUrl;
  url.replace("COIN", coin);

  HTTPClient http;
  StaticJsonDocument<2000> doc;
  DeserializationError error;
  int attempt = 0;

  while (attempt <= 2) {
    http.begin(url);
    http.GET();
    error = deserializeJson(doc, http.getString());
    if (!error) {
      break;
    } else if(error && attempt == 2) {
      return getErrorIfOccur(error);
    }
    delay(2000);
    attempt++;
  }

  double price = doc["lastPrice"].as<String>().toDouble();

  double percentChange = doc["priceChangePercent"].as<String>().toDouble();
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
    Serial.println("Inside if....");
    cancelText();
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
    vTaskDelay(20);
  }
}

void cancelText() {
  bool stopped = false;
  while (!stopped) {
    if (P.getZoneStatus(0)) {
      Serial.println("trying delete task..");
      vTaskDelete(taskHandle);
      stopped = true;
    }
  }
}
