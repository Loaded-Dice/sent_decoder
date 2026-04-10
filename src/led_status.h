#ifndef LED_STATUS_H
#define LED_STATUS_H

#include "config.h"
#include "sent_types.h"

void initStatusLed();
void updateStatusLed();
void triggerPingLedBlink();

#endif // LED_STATUS_H
