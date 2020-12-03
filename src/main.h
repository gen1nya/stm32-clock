#ifndef _MAIN_H
#define _MAIN_H

#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DS3231.h>

#define PIN_RTS_INT PA1
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET   -1 

void rtcInterrupt(void);
void sender(void * args);
void readFromRTC(void * args);

#endif