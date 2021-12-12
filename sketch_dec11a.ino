#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <WiFi.h>

#define MAX_DEVICES 4 //number of led matrix connect in series
#define CS_PIN 15
#define CLK_PIN 14
#define DATA_PIN 12

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

const char* ssid = "***REMOVED***";
const char* password = "***REMOVED***";

// SOFTWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

uint8_t scrollSpeed = 42;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 2000; // in milliseconds

void setup()
{
  P.begin();
  connectingWifi();
}

void connectingWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    display("Connecting...", 1000);
  }
  display("Connected!", 3000);
}

void display(char* text, long milis) {
  long startTime = millis();
  long elapsedTime = 0;
  while (elapsedTime < milis) {
    if (P.displayAnimate())
    {
      P.displayText(text, scrollAlign, scrollSpeed, 0, scrollEffect, scrollEffect);
    }
    elapsedTime = millis() - milis;
  }
}

void loop()
{
  display("Hello! Enter new message?", 1);
}
