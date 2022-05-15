#include <HTTPClient.h>

const String coinUrl = "https://api.binance.com/api/v3/ticker/24hr?symbol=COINUSDT";

String getData() {
  Data btc = getDataForCoin("BTC");
  Data eth = getDataForCoin("ETH");
  return eth.price + " ETH/USD " + "  " + eth.yesterdayChange + "%     " +  btc.price + " BTC/USD " + "  " + btc.yesterdayChange + "%     ";
}

Data getDataForCoin(String coin) {

  String url = coinUrl;
  url.replace("COIN", coin);

  HTTPClient http;
  StaticJsonDocument<2000> doc;
  DeserializationError error;
  int attempt = 0;

  while (attempt <= 3) {
    http.begin(url);
    http.GET();
    error = deserializeJson(doc, http.getString());
    if (!error) {
      break;
    } else if(error && attempt == 3) {
      return (Data) {"request",  "failed"};
    }
    delay(5000);
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
