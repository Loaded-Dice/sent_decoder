#include "misc_functions.h"
#include "output.h"

void resetSensor(){
  digitalWrite(ENABLE_PIN,LOW); 
  gpio_intr_disable(SENT_PIN);
  clearRingBuff();
  digitalWrite(ENABLE_PIN,LOW); 
  sig.supplyVoltage = false;
  sig.serialStatus = SENT_SER_NONE;
  sig.status = SIG_NONE;
  uartState = UART_NONE;
  unique = 0;
  memset(&uniqueIds, 0, sizeof(uniqueIds)); unique = 0;
  memcpy(&chFrame, &chDefault, sizeof(chFrame));
  memcpy(&sig, &sigDefault, sizeof(sig));
}

void startSensor(){
  if(digitalRead(ENABLE_PIN)) return;
  if(sig.ovc_protect_ms > millis()){ String msg ="#Reaktivierung möglich in " + String((sig.ovc_protect_ms - millis()) / 1000) + " Sekunden"; Serial.println(msg);}
  else{ gpio_intr_enable(SENT_PIN);  digitalWrite(ENABLE_PIN,HIGH); sig.supplyVoltage = true; sig.overcurrent = false; uartState = UART_NONE; clearRingBuff();}
}

void detectSignal(){
  uartState = UART_NONE;
  sig.serialStatus = SENT_SER_NONE;
  sig.status = SIG_NONE;
  unique = 0;
  memset(&uniqueIds, 0, sizeof(uniqueIds)); unique = 0;
  clearRingBuff();
}

bool isSync(BUFF_T * ptr0, BUFF_T * ptr1, uint32_t syncLen ){ return (IN_RANGE4(*ptr0,syncLen) && !IN_RANGE4(*ptr1,syncLen));}

int gotoFrameStart(uint16_t ringBuffSpan,uint16_t localTail) {
  int result = ringBuffSpan;
  portENTER_CRITICAL(&ringBufferMux);
  for (uint16_t i = 0; i < ringBuffSpan - sig.numPulses + 1; i++) {
    if (isSync((BUFF_T*)&ringBuffer.buffer[ringPos(localTail + i)],(BUFF_T*)&ringBuffer.buffer[ringPos(localTail + 1 + i)], sig.syncLen)){ result=i; break; }
  }
  portEXIT_CRITICAL(&ringBufferMux);
  return result;
}

int8_t getNibbleVal(BUFF_T pulseLen, uint32_t syncLen) {
    // SENT Spec 5.3.2: Rcal = Measured Calibration pulse period / (56 * Ticknom)
    // Data Value N = Round[(Measure pulse period N / Rcal - 12 * Ticknom) / Ticknom]
    // Simplified: Data Value N = Round[pulseLen * 56 / syncLen - 12]
    
    // Use fixed-point arithmetic for better precision: multiply by 56, add 0.5 for rounding, divide by syncLen, subtract 12
    int32_t nibble = (((int64_t)pulseLen * 56 + (syncLen / 2)) / syncLen) - 12;
    return CLAMP(nibble, 0, 15);
}

int8_t getNibbleVal_f(float pulseLen, float syncLen) {
    int nibble = round(pulseLen / (syncLen / 56.0)) - 12;
    return CLAMP(nibble,0,15);
}

int8_t calculateCRC4(int8_t data[], uint8_t numNibbles) {
    const uint8_t crc4_table[16] = {0, 13, 7, 10, 14, 3, 9, 4, 1, 12, 6, 11, 15, 2, 8, 5};
    uint8_t CheckSum16 = 5;
    for (uint8_t i = 0; i < numNibbles; i++) {
        CheckSum16 = uint8_t(data[i]) ^ crc4_table[CheckSum16];
    }
    CheckSum16 = 0 ^ crc4_table[CheckSum16];
    return CheckSum16;
}

uint8_t calculateCRC6(uint32_t crcEnhanced) {
    const uint8_t crc6_table[64] = {
    0, 25, 50, 43, 61, 36, 15, 22, 35, 58, 17, 8, 30, 7, 44, 53, 31, 6, 45, 52, 34, 59, 16, 9, 60, 37, 14, 23, 1, 24, 51, 42,
    62, 39, 12, 21, 3, 26, 49, 40, 29, 4, 47, 54, 32, 57, 18, 11, 33, 56, 19, 10, 28, 5, 46, 55, 2, 27, 48, 41, 63, 38, 13, 20 };
    crcEnhanced = (crcEnhanced << 6 & MASK_N_BITS(30));
    uint8_t checkSum = 21;
    for( uint i = 0; i < 5 ; i++){ checkSum = uint8_t((crcEnhanced >> ((4-i)*6) ) & MASK_N_BITS(6)) ^ crc6_table[checkSum]; } 
    return checkSum;
}

void printBin(uint32_t bin, uint8_t size){ for(int i = size-1; i >= 0; i--){ Serial.print((bin  >> i) & 1 ? '1' : '0');}}

uint8_t coarseFrameValid(BUFF_T * ptr) {
  BUFF_T c = ptr[0];
  BUFF_T tick = ptr[0] / 56;
  if ((c < 126 * ESP_TICKS_PER_US) || (tick < 2 * ESP_TICKS_PER_US)) return 0;

  uint8_t isNibble = 0;
  bool nextSync = false;
  uint8_t f_step = 0;
  for (uint8_t i = 1; i < 11; i++) {
    if (i < 5 && (ptr[i] / tick) >= 11 && (ptr[i] / tick) <= 28) { isNibble++; }
    else if (i >= 5 && IN_RANGE16(ptr[i], c) && !IN_RANGE16(ptr[i + 1], c)) {
      f_step = i;
      break;
    }
  }
  if (f_step != 0) { nextSync = isSync(&ptr[f_step], &ptr[f_step + 1], ptr[0]); }
  bool coarseValid = (f_step != 0 && isNibble >= 4 && nextSync);
  return coarseValid ? f_step : 0;
}

uint32_t get_f_len(BUFF_T * ptr, uint8_t f_pulses) {
  uint32_t sum_len = 0;
  for (int i = 0; i < f_pulses; i++) { sum_len += ptr[i]; }
  return sum_len;
}

void clearRingBuff(){
    portENTER_CRITICAL(&ringBufferMux);    
    ringBuffer.tail = ringBuffer.head;
    ringBuffer.overflow = false;
    portEXIT_CRITICAL(&ringBufferMux);
}

String getCodeAsHex(uint16_t code) {
    char hexString[7];
    sprintf(hexString, "0x%03X", code & 0xFFF);
    return String(hexString);
}
