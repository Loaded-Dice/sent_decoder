#include "led_status.h"

#include <FastLED.h>

namespace {

CRGB statusLed[1];

enum LedState : uint8_t {
  LED_BLUE = 0,
  LED_RED,
  LED_YELLOW,
  LED_GREEN
};

LedState getLedState() {
  if (!sig.supplyVoltage) {  return LED_BLUE;  }
  if (sig.serialStatus == SENT_SER_SETUP || sig.serialStatus == SENT_SER_COLLECT || sig.status == SIG_DETECT) {
    return LED_YELLOW;
  }
  if (sig.serialStatus == SENT_SER_LOOP && sig.status == SIG_OK) {    return LED_GREEN;  }
  if (sig.overcurrent || sig.status == SIG_NONE) {    return LED_RED;  }
  return LED_RED;
}

CRGB mapColor(LedState state) {
  switch (state) {
    case LED_BLUE:   return CRGB::Blue;
    case LED_RED:    return CRGB::Red;
    case LED_YELLOW: return CRGB::Yellow;
    case LED_GREEN:  return CRGB::Green;
    default:         return CRGB::Red;
  }
}

} // namespace

void initStatusLed() {
  FastLED.addLeds<WS2812B, LEDDATA_PIN, RGB>(statusLed, 1);
  FastLED.setBrightness(32);
  statusLed[0] = CRGB::Blue;
  FastLED.show();
}

void updateStatusLed() {
  static LedState lastState = static_cast<LedState>(255);

  const LedState newState = getLedState();
  if (newState == lastState) {
    return;
  }

  statusLed[0] = mapColor(newState);
  FastLED.show();
  lastState = newState;
}
