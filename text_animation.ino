#define MAX_DEVICES 10 //number of led matrix connect in series
#define CS_PIN 15
#define CLK_PIN 14
#define DATA_PIN 12

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define  BUF_SIZE  75

char curMessage[BUF_SIZE] = { "" };
char newMessage[BUF_SIZE] = { "" };
bool newMessageAvailable = true;

TaskHandle_t taskHandle = NULL;

// SOFTWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
void beginDisplaying() {
  P.begin();
  P.setIntensity(15);
  P.setScrollSpacing(9);
  
  if (taskHandle == NULL) {
    taskHandle = TaskHandle_t();
  }

  Serial.println("Creating task");
  xTaskCreate(
    animateText,
    "Task 1",
    1000,
    (void *) 1,
    1,
    &taskHandle
  );
}

void updateText(String text) {
  strcpy(newMessage, text.c_str());
  newMessageAvailable = true;
}

void animateText(void *param)
{
  P.displayText(curMessage, PA_LEFT, 45, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  for (;;) {
    if (P.displayAnimate()) // If finished displaying message
    {
      if (newMessageAvailable)
      {
        strcpy(curMessage, newMessage);
        newMessageAvailable = false;
      }
      P.displayReset();
    }
    vTaskDelay(10);
  }
}
