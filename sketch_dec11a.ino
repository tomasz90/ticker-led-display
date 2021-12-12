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

const char* ssid = "***REMOVED***";
const char* password = "***REMOVED***";

unsigned long interval = 86400000;
unsigned long nextUpdateTime = 0;
String yesterday = "";

// SOFTWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

uint8_t scrollSpeed = 48;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 2000; // in milliseconds

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
    Serial.print("updating time..");
    time_t nowS = DateTime.now();
    long int sec = 1639329382;
    Serial.print("Now seconds: ");
    Serial.print(sec);
    unsigned long timeTillUpdateMs = (interval - (sec % interval / 1000)) * 1000;
    nextUpdateTime = millis() + timeTillUpdateMs;
 
    unsigned long t = sec - interval / 1000;
    Serial.print(" Yesterday seconds: ");
    Serial.print(t);

    Serial.print(" Yesterday was: ");
    char buffer[40];
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
  StaticJsonDocument<2000> doc;
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

  String BTCUSDPrice = doc["ethereum"]["usd"].as<String>();                    //Store crypto price and update date in local variables
  http.end();
  
  Serial.print("ETH price: " + BTCUSDPrice);
  Serial.print("Getting history...");
  StaticJsonDocument<2000> historyDoc;
  http.begin(historyURL+date);                                                               //Get historical crypto price from API
  int historyHttpCode = http.GET();
  DeserializationError historyError = deserializeJson(historyDoc, http.getString());

  if (historyError) {                                                                   //Display error message if unsuccessful
    Serial.print(F("deserializeJson(History) failed"));
    Serial.println(historyError.f_str());
    delay(2500);
    return (Data) {
      "", ""
    };
  }

  Serial.print("History HTTP Status Code: ");
  Serial.println(historyHttpCode);
  JsonObject bpi = historyDoc["bpi"].as<JsonObject>();
  double yesterdayPrice;
  for (JsonPair kv : bpi) {
    yesterdayPrice = kv.value().as<double>();                                           //Store yesterday's crypto price
  }

  Serial.print("BTCUSD Price: ");                                                       //Display current price on serial monitor
  Serial.println(BTCUSDPrice.toDouble());

  Serial.print("Yesterday's Price: ");                                                  //Display yesterday's price on serial monitor
  Serial.println(yesterdayPrice);

  bool isUp = BTCUSDPrice.toDouble() > yesterdayPrice;                                  //Check whether price has increased or decreased
  double percentChange;
  String dayChangeString = "24hr. Change: ";
  if (isUp)                                                                             //If price has increased from yesterday
  {
    percentChange = ((BTCUSDPrice.toDouble() - yesterdayPrice) / yesterdayPrice) * 100;
  }
  else                                                                                  //If price has decreased from yesterday
  {
    percentChange = ((yesterdayPrice - BTCUSDPrice.toDouble()) / yesterdayPrice) * 100;
    dayChangeString = dayChangeString + "-";
  }

  Serial.print("Percent Change: ");                                                     //Display the percentage change on the serial monitor
  Serial.println(percentChange);
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
      P.displayText(text, scrollAlign, scrollSpeed, 0, scrollEffect, scrollEffect);
    }
    elapsedTime = millis() - startTime;
  }
}
