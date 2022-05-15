const String coinUrl = "https://api.binance.com/api/v3/ticker/24hr?symbol=COINUSDT";

Data getData(String coin) {

  String url = coinUrl;
  url.replace("COIN", coin);

  HTTPClient http;
  StaticJsonDocument<2000> doc;
  DeserializationError error;
  int attempt = 0;

  while (attempt <= 10) {
    http.begin(url);
    http.GET();
    error = deserializeJson(doc, http.getString());
    if (!error) {
      break;
    } else if(error && attempt == 10) {
      return getErrorIfOccur(error);
    }
    delay(30000);
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
