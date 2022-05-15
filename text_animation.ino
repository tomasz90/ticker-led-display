
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
