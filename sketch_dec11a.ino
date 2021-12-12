#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESPDateTime.h>
#include <TimeLib.h>

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

const int httpsPort = 443;                                                          //Bitcoin price API powered by CoinDesk - https://www.coindesk.com/price/bitcoin
const String url = "https://api.coingecko.com/api/v3/simple/price?ids=ethereum&vs_currencies=USD";
const String historyURL = "https://api.coingecko.com/api/v3/coins/ethereum/history?date=";

struct Data {
  String price;
  String yesterdayChange;
};

void setup()
{
  Serial.begin(115200);
  P.begin();
  connectingWifi();
}

void connectingWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    displayMsg("Connecting...", 1000);
  }
  displayMsg("Connected!", 5000);
}

void loop()
{
  String date = getCurrentDate();
  Data d = getData(date);
  String text = d.price + " BTC/USD " + "   " + d.yesterdayChange + "%";
  const char* msg = {text.c_str()};
  displayMsg(msg, 60000);
}

String getCurrentDate() {
  unsigned long currentTime = millis();
  if (nextUpdateTime - currentTime < 0 || nextUpdateTime - currentTime > interval) {
    time_t nowS = DateTime.now();
    long int sec = 1639329382;
    Serial.print("Now seconds: ");
    Serial.print(sec);
    unsigned long timeTillUpdateMs = (interval - (sec % interval / 1000)) * 1000;
    nextUpdateTime = millis() + timeTillUpdateMs;
    unsigned long t = sec - interval / 1000;
    Serial.print(" Yesterday was: ");
    char buffer[80];
    sprintf(buffer, "%02d-%02d-%04d", day(t), month(t), year(t));
    yesterday = String(buffer);
  }
  return yesterday;
}

Data getData(String date) {

  HTTPClient http;
  Serial.print("Connecting to ");                                                       //Display url on Serial monitor for debugging
  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();                                                            //Get crypto price from API
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, http.getString());

  if (error)                                                                            //Display error message if unsuccessful
  {
    Serial.print(F("deserializeJson Failed"));
    Serial.println(error.f_str());
    delay(2500);
    return (Data) {
      "", ""
    };
  }

  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);

  String BTCUSDPrice = doc["ethereum"]["usd"].as<String>();
  http.end();

  Serial.print("ETH price: " + BTCUSDPrice);
  Serial.print("Getting history...");
  DynamicJsonDocument historyDoc(16000);
  http.begin(historyURL + date);
  int historyHttpCode = http.GET();
  DeserializationError historyError = deserializeJson(historyDoc, http.getString());

  if (historyError) {
    Serial.print(F("deserializeJson(History) failed"));
    Serial.println(historyError.f_str());
    delay(2500);
    return (Data) {
      "", ""
    };
  }

  String yesterdayPrice = historyDoc["market_data"]["current_price"]["usd"].as<String>();
  http.end();

  Serial.print("BTCUSD Price: ");
  Serial.println(BTCUSDPrice.toDouble());

  Serial.print("Yesterday's Price: ");
  Serial.println(yesterdayPrice);

  double yPrice = yesterdayPrice.toDouble();
  double percentChange = (BTCUSDPrice.toDouble() - yPrice / yPrice) * 100;

  return (Data) {
    BTCUSDPrice, String(percentChange, 1)
  };
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
