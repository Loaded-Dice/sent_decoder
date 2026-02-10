#include "signal_processing.h"
#include "globals.h"
#include "misc_functions.h"
#include "analyze.h"
#include "output.h"

void processSignal() {
  static int lastHead = 0;
  uint16_t localTail, localHead;
  bool overflow;
  
  portENTER_CRITICAL(&ringBufferMux);
  localTail = ringBuffer.tail;
  localHead = ringBuffer.head;
  overflow = ringBuffer.overflow;
  portEXIT_CRITICAL(&ringBufferMux);
  
  if(localHead != lastHead){ sig.lastPulseTime_ms = millis();}

  uint32_t ringBuffSpan = getBuffSpan(localHead,localTail,RING_BUFFER_SIZE);

  if(sig.status == SIG_NONE && ringBuffSpan > 100){ sig.status = SIG_DETECT; clearRingBuff(); }

  else if(sig.status == SIG_DETECT){
    if(ringBuffSpan < DETECT_BUFFER_SIZE/2){
      if(millis() > sig.lastPulseTime_ms + 3000){ sig.status = SIG_NONE; clearRingBuff();}
      return;
    }
    else{
      uint16_t n_detect = CLAMP(ringBuffSpan,DETECT_BUFFER_SIZE/2,DETECT_BUFFER_SIZE);
      portENTER_CRITICAL(&ringBufferMux);    
      for(int i = 0; i < n_detect;i++){detect[i]= ringBuffer.buffer[ringPos(localTail+i)];}
      ringBuffer.tail = localTail;
      if(overflow) ringBuffer.overflow = false;
      portEXIT_CRITICAL(&ringBufferMux);
      if (processDetect(n_detect)) { sig.status = SIG_OK; outputSigCtrl();}
      else{ clearRingBuff(); sig.status = SIG_NONE; }
    }
  }

  else if(sig.status == SIG_OK){
    if(millis() > sig.lastPulseTime_ms + 3000){ sig.status = SIG_NONE; clearRingBuff();}
    while(ringBuffSpan >= sig.numPulses){
      uint16_t offset = gotoFrameStart(ringBuffSpan, localTail);
      count.pulseSkip += offset;
      localTail = ringPos(localTail + offset);
      ringBuffSpan -= offset;
      if(ringBuffSpan < sig.numPulses) break;
      BUFF_T pulseTimes[10];
      portENTER_CRITICAL(&ringBufferMux);
      for (uint8_t i = 0; i < sig.numPulses; i++) { pulseTimes[i] = ringBuffer.buffer[ringPos(localTail+i)]; }
      portEXIT_CRITICAL(&ringBufferMux);
      localTail = ringPos(localTail + sig.numPulses);
      ringBuffSpan -= sig.numPulses;
      sentFrame sentBuff = analyzeFramePulses(&pulseTimes[0],false);
      if(!sentBuff.crcOk){sentBuff = analyzeFramePulses(&pulseTimes[0],true);}
      if(sentBuff.crcOk){count.crcOk++;}
      else{ count.crcFail++; }
      count.frames++;
    }
    portENTER_CRITICAL(&ringBufferMux);    
    ringBuffer.tail = localTail;
    if(overflow) ringBuffer.overflow = false;
    portEXIT_CRITICAL(&ringBufferMux);
  }

  lastHead = localHead;
}
