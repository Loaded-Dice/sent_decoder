#include "analyze.h"
#include "globals.h"
#include "misc_functions.h"
#include "serial_comm.h"
#include "value_transfer.h"
#include "output.h"

// -----------------------------===================================={Analyze Signal Properties}====================================-----------------------------

void selectionSort(sortList* pl, uint16_t size, bool hi2lo, bool sort_idx) {
  for (uint16_t i = 0; i < size - 1; i++) {
    uint16_t maxIdx = i;
    for (uint16_t j = i + 1; j < size; j++) {
      if (sort_idx) {
        if (hi2lo && (pl[j].idx > pl[maxIdx].idx)) { maxIdx = j; }
        else if (!hi2lo && (pl[j].idx < pl[maxIdx].idx)) { maxIdx = j; }
      }
      else {
        if (hi2lo && (pl[j].val > pl[maxIdx].val)) { maxIdx = j; }
        else if (!hi2lo && (pl[j].val < pl[maxIdx].val)) { maxIdx = j; }
      }
    }
    if (maxIdx != i) { sortList temp = pl[i];  pl[i] = pl[maxIdx];  pl[maxIdx] = temp; }
  }
}

bool processDetect(uint16_t detectSize) {

  const uint8_t minSize = 50;
  if (detectSize < minSize + 11) return false;
  int16_t bestIdx = -1;
  uint16_t matchCount = 0;
  uint8_t f_pulses = 0;

  for (uint16_t n = 0; n < minSize; n++) {
    uint8_t f_step = coarseFrameValid((BUFF_T*)&detect[n]);
    if (f_step >= 6 && f_step <= 10) {
    uint16_t tmpMatches = 1;
    for (uint16_t i = 0; i < detectSize - f_step; i += f_step) {
      if (coarseFrameValid((BUFF_T*)&detect[n + i]) == f_step) {
        tmpMatches++;
        if (tmpMatches > matchCount) {
          matchCount = tmpMatches;
          bestIdx = n;
          f_pulses = f_step;
        }
      } 
      else { break; }
    }
    if (f_pulses && matchCount && ((detectSize / f_pulses) * 100 / matchCount) > 60){break;}
    }
  }

  if (f_pulses && matchCount && ((detectSize / f_pulses) * 100 / matchCount) > 60){
    sortList sl[matchCount];
    for (uint8_t i = 0; i < matchCount; i++) {    sl[i].val = detect[bestIdx + (i * f_pulses)];   sl[i].idx = i; }

    if(DEBUG){Serial.println(); for (uint8_t i = 0; i < matchCount; i++) {   Serial.print( sl[i].val ); Serial.print("\t");  } Serial.println();}

    selectionSort(&sl[0], matchCount);
    BUFF_T syncMedian = (sl[(matchCount / 2) + 1].val + sl[(matchCount / 2) + (matchCount % 2)].val) / 2;

    if(DEBUG){Serial.println(); for (uint8_t i = 0; i < matchCount; i++) {   Serial.print( sl[i].val ); Serial.print("\t");  } Serial.println();}
    // set signal control variables
    sig.syncLen = syncMedian;
    sig.tickLen = syncMedian/56;
    sig.syncNominal_us = round((float)syncMedian / (float)ESP_TICKS_PER_US);
    sig.tickNominal_us = round((float)sig.tickLen  / ESP_TICKS_PER_US);
    sig.syncNominal_ticks = sig.syncNominal_us * ESP_TICKS_PER_US;
    sig.tickNominal_ticks = sig.tickNominal_us * ESP_TICKS_PER_US;

    // Plausibility check: rounded tick nominal must reconstruct sync within 15%.
    // Catches false detections at half/double the real sync length (e.g. 84.7µs instead of 169µs).
    uint32_t expectedSyncTicks = (uint32_t)sig.tickNominal_ticks * 56;
    int32_t syncError = (int32_t)expectedSyncTicks - (int32_t)syncMedian;
    if (syncError < 0) syncError = -syncError;
    if (syncError * 100 > (int32_t)syncMedian * 15) return false;

    sig.frameLen = 0;
    for (uint8_t i = 0; i < f_pulses; i++) { sig.frameLen += detect[bestIdx + i]; }
    sig.numPulses = f_pulses;
    BUFF_T iMid = f_pulses * (matchCount / 2) + bestIdx;
    sig.frameLenConst = (IN_RANGE16(detect[iMid], detect[iMid + f_pulses]) && IN_RANGE16(detect[iMid], detect[iMid + f_pulses]));
    sig.dataNibbles = f_pulses - (sig.frameLenConst + 3);
    uint32_t maxLen1 = (sig.tickLen * 768) + ((sig.tickLen * 768) >> 2);
    uint32_t maxLen2 = (sig.frameLen + sig.frameLen) >> 2;
    sig.maxLen = maxLen1 > maxLen2 ? maxLen2 : maxLen1;
    sig.minLen = sig.tickLen * 12;
    sig.minLen -= (sig.minLen >> 2);
    return true;
  }
  return false;
}

// -----------------------------===================================={Analyze Frame Impulses}====================================-----------------------------

sentFrame analyzeFramePulses(BUFF_T * ptr, bool localSync){

  sentFrame frame;
  frame.syncLen = ptr[0];
  frame.syncOk = IN_RANGE4(frame.syncLen, sig.syncLen);
  
   for (uint8_t i = 0; i < sig.numPulses; i++) { 
    frame.frameLen += ptr[i]; 
    if(i >= 1 && i < sig.numPulses - sig.frameLenConst){
      int8_t value = getNibbleVal(ptr[i], localSync ? ptr[0] : sig.syncLen);
      if(i == 1){ frame.status = value; }
      if(i > 1 && i < sig.dataNibbles + 2){frame.data[i-2] = value;}
      if(i == sig.dataNibbles + 2){ frame.crc = value; }
    }
  }
  frame.crcOk = (frame.crc == calculateCRC4(&frame.data[0], sig.dataNibbles) );  
  frame.lenOk = sig.frameLenConst ? IN_RANGE4(frame.frameLen, sig.frameLen) : true;
  collectSerialMsg(frame.status);

    if(sig.serialStatus == SENT_SER_LOOP && frame.crcOk){ 
       parseToCh(frame); 

       if(chFrame.ch1.type    != ch_undefined){rawTransfer(&chFrame.ch1);}
       if(chFrame.ch2.type     != ch_undefined){rawTransfer(&chFrame.ch2);}
       if(chFrame.ch_supp[0][0].type != ch_undefined){rawTransfer(&chFrame.ch_supp[0][0]);}
     }
  return frame;
}

static bool s_resetCollectSerial = false;

void resetCollectSerialMsg() {
  s_resetCollectSerial = true;
}

void collectSerialMsg(uint8_t status) {

  static uint32_t bit2 = 0;
  static uint32_t bit3 = 0;
  bool newSentSerial = false;
  static uint32_t crcEnhanced = 0;

  if (s_resetCollectSerial) { bit2 = 0; bit3 = 0; crcEnhanced = 0; s_resetCollectSerial = false; }
  
  bit2 = (bit2 << 1) | ((status >> 2) & 0x01);
  bit3 = (bit3 << 1) | ((status >> 3) & 0x01);
  crcEnhanced = (crcEnhanced << 2) | (((status >> 2) & 0x01) << 1) | ((status >> 3) & 0x01);

  if((bit3 & MASK_N_BITS(16)) == 32768){lastSentSerial = extractShort(bit2); newSentSerial = true;}

  bool configBit = bool((bit3 >> 10) & 1);
  bool isEnhanced = ((bit3 >> 11) & MASK_N_BITS(7)) == 0b01111110;
  if (isEnhanced){lastSentSerial = extractEnhanced(bit3,bit2,configBit,crcEnhanced); newSentSerial = true;}

  if(newSentSerial){
    if(sig.serialStatus == SENT_SER_NONE){ sig.serialStatus = SENT_SER_COLLECT; unique = 0; infoMsg("Analyzing SENT Serial Msg"); }
    if(     sig.serialStatus == SENT_SER_COLLECT && lastSentSerial.isValid){ collectSentSerialCycle(lastSentSerial); }
    else if(sig.serialStatus == SENT_SER_SETUP){parseUniqueIdList(); }
    else if(sig.serialStatus == SENT_SER_LOOP){parseSerialSent(lastSentSerial);}
    }    
}

void rawTransfer(Channel * ch){
  if     (ch->type == ch_pressure_lin || ch->type == ch_MAF_lin || ch->type == ch_ratio_encoding || ch->type == ch_position_lin_angle || ch->type == ch_position_lin ){linearTransfer(ch);}
  else if(  ch->type == ch_temperature_lin_special || ch->type == ch_temperature_lin_default || ch->type == ch_temperature_lin_high ){linearTemperatureTransfer(ch);}
  else{
    return;
  }
}

void parseToCh(sentFrame raw){
    if (chFrame.ch1.depth == 12 && chFrame.ch2.depth == 12 && chFrame.nibbles == 6 && chFrame.ch2.type != ch_secure ){
        chFrame.ch1.raw = mergeNibbles(raw.data[0],raw.data[1],raw.data[2]); chFrame.ch1.statErr = (bool)(raw.status & 1);
        chFrame.ch2.raw = mergeNibbles(raw.data[5],raw.data[4],raw.data[3]); chFrame.ch2.statErr = (bool)((raw.status >> 1) & 1) ;
        error.specMissmatch = (chFrame.ch2.type == ch_zero && chFrame.ch2.raw > 0);
    }
    else if(chFrame.ch1.depth == 12 && chFrame.nibbles == 3    && chFrame.ch2.type == ch_undefined){
        chFrame.ch1.raw = mergeNibbles(raw.data[0],raw.data[1],raw.data[2]); chFrame.ch1.statErr = (bool)(raw.status & 1);
    }
    else if(chFrame.ch1.depth == 12 && chFrame.nibbles == 4    && chFrame.ch2.type == ch_undefined){
        chFrame.ch1.raw = combineNibblesHighSpeed(raw.data); chFrame.ch1.statErr = (bool)(raw.status & 1);
    }
    else if(chFrame.ch1.depth == 12 && chFrame.ch2.depth == 12 && chFrame.nibbles == 6 && chFrame.ch2.type == ch_secure ){
        chFrame.ch1.raw = mergeNibbles(raw.data[0],raw.data[1],raw.data[2]); chFrame.ch1.statErr = (bool)(raw.status & 1);
        chFrame.ch2.raw = mergeNibbles(raw.data[3],raw.data[4]);
        chFrame.ch2.statErr = chkSecureCh(raw.data[0],raw.data[5],mergeNibbles(raw.data[3],raw.data[4]));
    }
    else if(chFrame.ch1.depth == 14 && chFrame.ch2.depth == 10 && chFrame.nibbles == 6){
        combineNibbles_14bit_10bit(raw.data, &chFrame.ch1.raw, &chFrame.ch2.raw);
        chFrame.ch1.statErr = (bool)(raw.status & 1); chFrame.ch2.statErr = (bool)((raw.status >> 1) & 1);
    }
    else if(chFrame.ch1.depth == 16 && chFrame.ch2.depth == 8  && chFrame.nibbles == 6){
        chFrame.ch1.raw = mergeNibbles(raw.data[0],raw.data[1],raw.data[2],raw.data[3]); chFrame.ch1.statErr = (bool)(raw.status & 1);
        chFrame.ch2.raw = mergeNibbles(raw.data[4],raw.data[5]);
    }
    else{ error.parse = true; }
}

bool chkSecureCh(int8_t firstNibble, int8_t lastNibble, uint8_t count){
    if (lastNibble < 0 || lastNibble > 15 || firstNibble < 0 || firstNibble > 15) { error.parse = true; return true; }
    if(count != secure.counter){secure.missed++;}
    else{secure.missed = 0;}
    deltaRoll = secure.counter - count;
    secure.invertMatch = (15-firstNibble == lastNibble);
    secure.counter = count;
    secure.counter++;
    return (secure.missed > 6);
}

uint16_t mergeNibbles(int8_t n0, int8_t n1) { return ((n0 & 0xF) << 4) | (n1 & 0xF);}

uint16_t mergeNibbles(int8_t n0, int8_t n1, int8_t n2) { return ((n0 & 0xF) << 8) | ((n1 & 0xF) << 4) | (n2 & 0xF);}

uint16_t mergeNibbles(int8_t n0, int8_t n1, int8_t n2, int8_t n3) { return ((n0 & 0xF) << 12) | ((n1 & 0xF) << 8) | ((n2 & 0xF) << 4) | (n3 & 0xF);}

uint16_t combineNibbles(int8_t nibbles[], uint8_t length) {
    int32_t result = 0;
    if (length == 0) { error.parse = true; return 0; }
    for (uint8_t i = 0; i < length; i++) {
        if (nibbles[i] < 0 || nibbles[i] > 15) { error.parse = true; return 0;}
        result = (result << 4) | (uint8_t)nibbles[i];
        if (result > 0xFFFF) {error.parse = true; return 0; }
    }
    return result;
}

uint16_t combineNibblesHighSpeed(int8_t nibbles[]) {
    uint16_t result = 0;
    for (uint8_t i = 0; i < 4; i++) {
        if (nibbles[i] < 0 || nibbles[i] > 7) { error.parse = true; return 0;}
        result |= (((uint8_t)nibbles[i] >> 2) & 0x01) << (11-(i*3));
        result |= (((uint8_t)nibbles[i] >> 1) & 0x01) << (10-(i*3));
        result |= (((uint8_t)nibbles[i] >> 0) & 0x01) << ( 9-(i*3));
    }
  return result;
}

void combineNibbles_14bit_10bit(int8_t nibbles[], uint16_t *rawCh1, uint16_t *rawCh2) {
    uint16_t result1 = 0;
    uint16_t result2 = 0;
    for (uint8_t i = 0; i < 6; i++) { if (nibbles[i] < 0 || nibbles[i] > 7) { error.parse = true; return ;} }
    result1 |= ((uint16_t)nibbles[0] & 0x0F) << 10;
    result1 |= ((uint16_t)nibbles[1] & 0x0F) << 6;
    result1 |= ((uint16_t)nibbles[2] & 0x0F) << 2;
    result1 |= (((uint16_t)nibbles[3] >> 3) & 0x01) << 1;
    result1 |= ((uint16_t)nibbles[3] >> 2) & 0x01;
    result2 |= ((uint16_t)nibbles[3] & 0x03);
    result2 |= ((uint16_t)nibbles[4] & 0x0F) << 2;
    result2 |= ((uint16_t)nibbles[5] & 0x0F) << 6;
    *rawCh1 = result1;
    *rawCh2 = result2;
}
