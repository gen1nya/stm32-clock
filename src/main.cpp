#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <RTClib.h> 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

struct DateTimeStuct{
  uint8_t hours = 0;
  uint8_t min = 0;
  uint8_t sec = 0;
};

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET   -1 

RTC_DS3231 rtc;
DateTimeStuct now;

QueueHandle_t timeQueueHandler = xQueueCreate(1, sizeof now);

void readTimeFromRTC(void* arg) {
  UNUSED(arg);
  
  digitalWrite(LED_BUILTIN, HIGH);  
  vTaskDelay(100/ portTICK_PERIOD_MS);
  digitalWrite(LED_BUILTIN, LOW);
  static DateTimeStuct now;
  now.sec = 50;
  //pinMode(PB6, OUTPUT);
  //pinMode(PB7, OUTPUT);
  //rtc.begin();
  for(;;){
    /*DateTime _now = rtc.now();
    uint8_t s = _now.second();
    memcpy(&now.sec, &s, 1);
    now.hours = _now.hour();
    now.min = 10;
    //now.sec = _now.second();*/
    digitalWrite(LED_BUILTIN, HIGH);
    xQueueSend(timeQueueHandler, &now, 0);
    vTaskDelay(100/ portTICK_PERIOD_MS);
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void print(void* arg) {
  UNUSED(arg);
  
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setRotation(2);
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  display.setTextSize(2); 
  display.setTextColor(SSD1306_WHITE); 
  int i = 0;


  for(;;) {
    if (uxQueueMessagesWaiting(timeQueueHandler) != 0) {
      display.clearDisplay();
      display.setCursor(0,0);
      xQueueReceive(timeQueueHandler, &now, 0);
      display.print(now.hours);
      display.print(now.min);
      display.print(now.hours);
      display.print(++i);
      display.print("\nfuck");
      display.display();
    }
    //display.printf("%02d:%02d:%02d", now.hours, now.min, now.sec);
    
    vTaskDelay(32 / portTICK_PERIOD_MS);
  }
}

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  //pinMode(LED_BUILTIN, OUTPUT);
  xTaskCreate(readTimeFromRTC, "readTimeFromRTC", 3000, NULL, 1, NULL);
  //xTaskCreate(print, "print", 3000, NULL, 2, NULL);

  vTaskStartScheduler();
  
  while(1);
}

void loop() { }