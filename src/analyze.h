#ifndef ANALYZE_H
#define ANALYZE_H

#include "config.h"
#include "sent_types.h"
#include "value_transfer.h"

// Signal detection and analysis
void selectionSort(sortList* pl, uint16_t size, bool hi2lo = true, bool sort_idx = false);
bool processDetect(uint16_t detectSize);

// Frame analysis
sentFrame analyzeFramePulses(BUFF_T* ptr, bool localSync);
void collectSerialMsg(uint8_t status);
void parseToCh(sentFrame raw);

// Nibble combining functions
uint16_t mergeNibbles(int8_t n0, int8_t n1);
uint16_t mergeNibbles(int8_t n0, int8_t n1, int8_t n2);
uint16_t mergeNibbles(int8_t n0, int8_t n1, int8_t n2, int8_t n3);
uint16_t combineNibbles(int8_t nibbles[], uint8_t length);
uint16_t combineNibblesHighSpeed(int8_t nibbles[]);
void combineNibbles_14bit_10bit(int8_t nibbles[], uint16_t *rawCh1, uint16_t *rawCh2);
bool chkSecureCh(int8_t firstNibble, int8_t lastNibble, uint8_t count);

// Value transfer
void rawTransfer(Channel* ch);

#endif // ANALYZE_H
