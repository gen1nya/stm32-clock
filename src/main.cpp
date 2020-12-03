#include "main.h"

QueueHandle_t timeQueueHandler;
SemaphoreHandle_t rtcIntSemaphore;

void readFromRTC(void * args) {
  static RTCDateTime now;
  DS3231 rtc;
  rtc.begin();
  rtc.setOutput(DS3231_1HZ);

  for (;;) {
    xSemaphoreTake(rtcIntSemaphore, portMAX_DELAY);
    now = rtc.getDateTime();
    xQueueSend(timeQueueHandler, &now, 0);
  }
}

void print(void * args) {
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  
  RTCDateTime _now;
  
  timeQueueHandler = xQueueCreate(1, sizeof _now);
  portTickType xLastWakeTime = xTaskGetTickCount();
  
  uint8_t pixel = 0;

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setRotation(2);
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();
  display.setTextSize(2); 
  display.setTextColor(SSD1306_WHITE); 
  display.clearDisplay();
  
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  portBASE_TYPE senderTask = xTaskCreate(readFromRTC, "readFromRTC", 256, NULL, 1, NULL);

  if (senderTask == pdTRUE) {
    display.println("task ok");
  } else {
    display.println("error(");
  }
  
  display.display();
  
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  
  for(;;) {
    if (uxQueueMessagesWaiting(timeQueueHandler) != 0) {
      xQueueReceive(timeQueueHandler, &_now, 0);
    }
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("%02d:%02d:%02d", _now.hour, _now.minute, _now.second);
  
    pixel++;
    if (pixel >= 128) pixel = 0;

    display.drawPixel(pixel, 31, WHITE);
    display.display();

    vTaskDelayUntil( &xLastWakeTime, ( 64 / portTICK_RATE_MS ) );
  }
}

void setup() {

  xTaskCreate(print, "print", 256, NULL, 2, NULL);
  rtcIntSemaphore = xSemaphoreCreateCounting(1, 0);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_RTS_INT, INPUT_PULLUP);
  attachInterrupt(PIN_RTS_INT, rtcInterrupt, FALLING);
  vTaskStartScheduler();
  
  for(;;);
}

void loop() { }

volatile int state3 = LOW;

void rtcInterrupt() {
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(rtcIntSemaphore, &xHigherPriorityTaskWoken);
  state3 = !state3;
  digitalWrite(LED_BUILTIN, state3);
}