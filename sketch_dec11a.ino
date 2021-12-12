#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define MAX_DEVICES 4 //number of led matrix connect in series
#define CS_PIN 15
#define CLK_PIN 14
#define DATA_PIN 12

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

// SOFTWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

uint8_t scrollSpeed = 42;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 2000; // in milliseconds

// Global message buffers shared by Serial and Scrolling functions
#define  BUF_SIZE  75
char newMessage[BUF_SIZE] = { "Hello! Enter new message?XD" };

void setup()
{
  P.begin();
}

void loop()
{
  if (P.displayAnimate())
  {
    P.displayText(newMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
  }
}
