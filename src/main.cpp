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
  RTCDateTime now;
  
  timeQueueHandler = xQueueCreate(1, sizeof now);
  portTickType xLastWakeTime = xTaskGetTickCount();
  
  uint8_t pixel = 0;
  uint8_t offset = 0;
  bool direction = true;

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setRotation(2);
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();
  display.setTextSize(2); 
  display.setTextColor(SSD1306_WHITE); 
  display.clearDisplay();
  
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  xTaskCreate(readFromRTC, "readFromRTC", 256, NULL, 1, NULL);

  display.display();
  
  vTaskDelay(100 / portTICK_PERIOD_MS);
  
  for(;;) {
    if (uxQueueMessagesWaiting(timeQueueHandler) != 0) {
      xQueueReceive(timeQueueHandler, &now, 0);
    }
 
    direction ? offset++ : offset--;
    if (offset > 30) direction = false;
    if (offset == 0) direction = true;
  
    display.clearDisplay();
    display.setCursor(offset,0);
    display.printf("%02d:%02d:%02d", now.hour, now.minute, now.second);
  
    if (pixel++ >= SCREEN_WIDTH) pixel = 0;
      
    display.drawPixel(pixel, SCREEN_HEIGHT - 1, WHITE);
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

volatile int ledState = LOW;

void rtcInterrupt() {
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(rtcIntSemaphore, &xHigherPriorityTaskWoken);
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState);
}