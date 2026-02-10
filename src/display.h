#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include "sent_types.h"
#include "img.h"

#ifdef DISPLAY_ACTIVE

// TFT main functions
void tftMain();
void set_img();
void set_img(bool init);
void chkTft();
void displayIdentifier();

// Menu functions
void setMenuPos(uint8_t* selectPos, uint8_t* lineOff, int8_t posDelta, uint8_t arraySize, uint8_t maxLines);
void drawMenu();
void drawMenu(bool init);
void menuBtnHandler(uint8_t idx);

// Helper functions
uint16_t getEntryColor(uint8_t idx);
String getEntryValue(uint8_t idx);
String getSignalStatus();
String getSerialStatus();

#endif // DISPLAY_ACTIVE

#endif // DISPLAY_H
