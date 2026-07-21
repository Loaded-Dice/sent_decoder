#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "soc/gpio_struct.h"
#include "soc/gpio_reg.h"
#include "driver/gpio.h"
#include <driver/pulse_cnt.h>
#include <FastLED.h>
#include <ArduinoJson.h>

#define SIZE(A) sizeof(A) / sizeof(A[0])
#define ABSDELTA(a, b) ((a) > (b) ? (a) - (b) : (b) - (a))
#define IN_RANGE(x, low, high) (((x) >= (low)) && ((x) <= (high)))
#define IN_RANGE64(x, c) (((x) >= (c - (c >> 6))) && ((x) <= (c + (c >> 6))))
#define IN_RANGE32(x, c) (((x) >= (c - (c >> 5))) && ((x) <= (c + (c >> 5))))
#define IN_RANGE16(x, c) (((x) >= (c - (c >> 4))) && ((x) <= (c + (c >> 4))))
#define IN_RANGE8(x, c) (((x) >= (c - (c >> 3))) && ((x) <= (c + (c >> 3))))
#define IN_RANGE4(x, c) (((x) >= (c - (c >> 2))) && ((x) <= (c + (c >> 2))))
#define CLAMP(x, low, high) ((x) < (low) ? (low) : (x) > (high) ? (high) : (x))
#define MASK_N_BITS(n) ((uint64_t(1) << (n)) - 1)

#define SENT_PIN GPIO_NUM_4
#define ENABLE_PIN 27
#define FAULT_PIN 26
#define RING_BUFFER_SIZE 8192
#define REC_BUFFER_SIZE 65536
#define DETECT_BUFFER_SIZE 300
#define MAX_UNIQUE 64
#define DEBUG false
#define IDENTIFY "DECODER 1"
#define UART_BAUD 250000

#define ESP_TICKS_PER_US 40
#define TIMER_CLK_DIV 2

#define AUTO_ENABLE_SENSOR false
#define VCC_WARMUP_MS 100
#define LEDDATA_PIN 23
#define ENCA_PIN 39
#define ENCB_PIN 34
#define ENCBTN_PIN GPIO_NUM_36
#define JSON_OUTPUT true
#define DISPLAY_ACTIVE

#ifdef DISPLAY_ACTIVE
#include <SPI.h>
#include <TFT_eSPI.h>
extern TFT_eSPI tft;
extern TFT_eSprite spr_valch1;
extern TFT_eSprite spr_valch2;
extern TFT_eSprite spr_valsupp;
#endif

#endif // CONFIG_H
