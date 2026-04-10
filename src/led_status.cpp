#include "led_status.h"

#include <FastLED.h>

namespace {

CRGB statusLed[1];
bool pingBlinkActive = false;
uint8_t pingBlinkStep = 0;
uint32_t pingBlinkNext_ms = 0;
CRGB pingRestoreColor = CRGB::Black;

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
  const uint32_t now = millis();

  if (pingBlinkActive) {
    if (now >= pingBlinkNext_ms) {
      const bool ledOn = (pingBlinkStep % 2) == 0;
      statusLed[0] = ledOn ? CRGB(180, 0, 200) : CRGB::Black;
      FastLED.show();
      pingBlinkStep++;

      if (pingBlinkStep >= 6) {
        pingBlinkActive = false;
        statusLed[0] = pingRestoreColor;
        FastLED.show();
      } else {
        pingBlinkNext_ms = now + (ledOn ? 90 : 70);
      }
    }
    return;
  }

  const LedState newState = getLedState();
  if (newState == lastState) {
    return;
  }

  statusLed[0] = mapColor(newState);
  FastLED.show();
  lastState = newState;
}

void triggerPingLedBlink() {
  pingRestoreColor = statusLed[0];
  pingBlinkActive = true;
  pingBlinkStep = 0;
  pingBlinkNext_ms = millis();
}
