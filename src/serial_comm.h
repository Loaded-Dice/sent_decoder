#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include "config.h"
#include "sent_types.h"

// SENT Serial message extraction
SentSerialMessage extractEnhanced(uint32_t bit3, uint32_t bit2, bool config, uint32_t crcEnhanced);
SentSerialMessage extractShort(uint16_t shortMsg);

// Serial message collection and parsing
void collectSentSerialCycle(SentSerialMessage msg);
void parseUniqueIdList();
void parseSerialSent(SentSerialMessage msg);
void decodeSentAscii(uint8_t messageId, uint16_t code);

// Utility functions
void bubbleSort(IDlist arr[], uint8_t n);

#endif // SERIAL_COMM_H
