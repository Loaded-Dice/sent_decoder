#ifndef MISC_FUNCTIONS_H
#define MISC_FUNCTIONS_H

#include "config.h"
#include "sent_types.h"

// Sensor control
void resetSensor();
void startSensor();
void detectSignal();
void clearRingBuff();
String getCodeAsHex(uint16_t code);

// Ring buffer utilities
inline uint32_t ringPos(uint32_t idx) {return idx & (RING_BUFFER_SIZE - 1);}
inline uint16_t getBuffSpan(uint16_t head, uint16_t tail, uint16_t buffSize) {return (head >= tail) ? head - tail : buffSize - tail + head;}
void clearRingBuff();

// Signal analysis utilities
bool isSync(BUFF_T* ptr0, BUFF_T* ptr1, uint32_t syncLen);
int gotoFrameStart(uint16_t ringBuffSpan, uint16_t localTail);
uint8_t coarseFrameValid(BUFF_T* ptr);

// Nibble and CRC calculations
int8_t getNibbleVal(BUFF_T pulseLen, uint32_t syncLen);
int8_t getNibbleVal_f(float pulseLen, float syncLen);
int8_t calculateCRC4(int8_t data[], uint8_t numNibbles);
uint8_t calculateCRC6(uint32_t crcEnhanced);

// Debug utilities
void printBin(uint32_t bin, uint8_t size);

String getValAsHex(uint32_t val);
String getValAs0xHex(uint32_t val);
String getValAsHex(uint32_t val, uint8_t bit);
String getValAs0xHex(uint32_t val, uint8_t bit);
String getValAsHex(uint32_t val, uint8_t bit, bool prefix);

#endif // MISC_FUNCTIONS_H
