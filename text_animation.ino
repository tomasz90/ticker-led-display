
void displayText(const void *text) {

  if (taskHandle != NULL) {
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
  P.displayText((char*)param, PA_LEFT, 45, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  for (;;) {
    if (P.displayAnimate()) // If finished displaying message
    {
      P.displayReset();
    }
    vTaskDelay(10);
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
