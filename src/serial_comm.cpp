#include "serial_comm.h"
#include "globals.h"
#include "misc_functions.h"
#include "channel_config.h"
#include "value_transfer.h"
#include "output.h"

SentSerialMessage extractEnhanced(uint32_t bit3, uint32_t bit2, bool config, uint32_t crcEnhanced) {
  
    SentSerialMessage newMsg;
    if (!config) { newMsg.messageId = (((bit3 >> 6) & 0x0F) << 4 ) | ((bit3 >> 1) & 0x0F); }
    else { newMsg.messageId = (bit3 >> 6) & 0x0F; newMsg.data = ((bit3 >> 1) & 0x0F) << 12; }       
    newMsg.data |= (bit2 & 0xFFF);
    newMsg.crc = (bit2 >> 12) & 0x3F;
    newMsg.isValid = (newMsg.crc == calculateCRC6(crcEnhanced));
    return newMsg;
}

SentSerialMessage extractShort(uint16_t shortMsg) {
    SentSerialMessage newMsg;
    newMsg.messageId = (shortMsg >> 12) & 0x0F;
    newMsg.data = (shortMsg >> 4) & 0xFF;
    newMsg.crc = shortMsg & 0x0F;
    return newMsg;
}

const uint8_t min_repeats = 3;
const uint8_t min_consecutive = 10;

void collectSentSerialCycle(SentSerialMessage msg){
if(!msg.isValid){return;}
static uint8_t consecutive = 0; 
int16_t foundIdx = -1;
  if(!msg.isValid){return;}
  for (uint8_t i = 0; i < unique; i++){  if( uniqueIds[i].messageId == msg.messageId){ foundIdx = i; break; }}

  if(foundIdx != -1){
    if(uniqueIds[foundIdx].code != msg.data){uniqueIds[foundIdx].fixedVal = false;}
    uniqueIds[foundIdx].code = msg.data ; uniqueIds[foundIdx].repeats++; consecutive++; 
    }
  else if(foundIdx == -1 && unique < MAX_UNIQUE){
       uniqueIds[unique].messageId = msg.messageId; uniqueIds[unique].code = msg.data ; 
       consecutive = 0;
       unique++;
    }
  bool cycleDone = true;  
  for (uint8_t i = 0; i < unique; i++){  if( uniqueIds[i].repeats < min_repeats ){cycleDone = false;  break; } }
  if(cycleDone && consecutive > min_consecutive){ sig.serialStatus = SENT_SER_SETUP; consecutive = 0; }
}

void bubbleSort(IDlist arr[], uint8_t n) {
    for (uint8_t i = 0; i < n - 1; i++) {
        for (uint8_t j = 0; j < n - i - 1; j++) {
            if (arr[j].messageId > arr[j + 1].messageId) {
                IDlist temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void parseUniqueIdList(){
    for (uint8_t i = 0; i < unique; i++){

    switch(uniqueIds[i].messageId) {
        case 0x00: break;
        case 0x01: sentMeta.status = uniqueIds[i].code; uniqueIds[i].assigned = true; break;
        case 0x02: break;
        case 0x03: sentMeta.chTypes = uniqueIds[i].code; setChConfig(&chFrame, uniqueIds[i].code); uniqueIds[i].assigned = true; break;
        case 0x04: sentMeta.configCode = uniqueIds[i].code; uniqueIds[i].assigned = true; break;
        case 0x05: sentMeta.manufacturer = uniqueIds[i].code; uniqueIds[i].assigned = true; break;
        case 0x06: sentMeta.sentRev = uniqueIds[i].code; uniqueIds[i].assigned = true; break;
        case 0x07: chFrame.ch1.nodes.x1 = uniqueIds[i].code; chFrame.ch1.nodes.x1done = true; uniqueIds[i].assigned = true; break;
        case 0x08: chFrame.ch1.nodes.x2 = uniqueIds[i].code; chFrame.ch1.nodes.x2done = true; uniqueIds[i].assigned = true; break;
        case 0x09: chFrame.ch1.nodes.y1 = uniqueIds[i].code; chFrame.ch1.nodes.y1done = true; uniqueIds[i].assigned = true; break;
        case 0x0A: chFrame.ch1.nodes.y2 = uniqueIds[i].code; chFrame.ch1.nodes.y2done = true; uniqueIds[i].assigned = true; break;
        case 0x0B: chFrame.ch2.nodes.x1 = uniqueIds[i].code; chFrame.ch2.nodes.x1done = true; uniqueIds[i].assigned = true; break;
        case 0x0C: chFrame.ch2.nodes.x2 = uniqueIds[i].code; chFrame.ch2.nodes.x2done = true; uniqueIds[i].assigned = true; break;
        case 0x0D: chFrame.ch2.nodes.y1 = uniqueIds[i].code; chFrame.ch2.nodes.y1done = true; uniqueIds[i].assigned = true; break;
        case 0x0E: chFrame.ch2.nodes.y2 = uniqueIds[i].code; chFrame.ch2.nodes.y2done = true; uniqueIds[i].assigned = true; break;
        case 0x0F: break;
        case 0x10: chFrame.ch_supp[0][0].raw = uniqueIds[i].code; uniqueIds[i].assigned = true; break;
        case 0x11: chFrame.ch_supp[0][1].raw = uniqueIds[i].code; uniqueIds[i].assigned = true; break;
        case 0x12: chFrame.ch_supp[0][0].nodes.x1 = uniqueIds[i].code; chFrame.ch_supp[0][0].nodes.x1done = true; 
                   chFrame.ch_supp[0][1].nodes.x1 = uniqueIds[i].code; chFrame.ch_supp[0][1].nodes.x1done = true;
                   uniqueIds[i].assigned = true; break;
        case 0x13: chFrame.ch_supp[0][0].nodes.x2 = uniqueIds[i].code; chFrame.ch_supp[0][0].nodes.x2done = true; 
                   chFrame.ch_supp[0][1].nodes.x2 = uniqueIds[i].code; chFrame.ch_supp[0][1].nodes.x2done = true; 
                   uniqueIds[i].assigned = true; break;
        case 0x14: chFrame.ch_supp[0][0].nodes.y1 = uniqueIds[i].code; chFrame.ch_supp[0][0].nodes.y1done = true; 
                   chFrame.ch_supp[0][1].nodes.y1 = uniqueIds[i].code; chFrame.ch_supp[0][1].nodes.y1done = true;
                   uniqueIds[i].assigned = true; break;
        case 0x15: chFrame.ch_supp[0][0].nodes.y2 = uniqueIds[i].code; chFrame.ch_supp[0][0].nodes.y2done = true; 
                   chFrame.ch_supp[0][1].nodes.y2 = uniqueIds[i].code; chFrame.ch_supp[0][1].nodes.y2done = true;
                   uniqueIds[i].assigned = true; break;

        case 0x16: chFrame.ch_supp[1][0].raw = uniqueIds[i].code; uniqueIds[i].assigned = true; break;
        case 0x17: chFrame.ch_supp[1][1].raw = uniqueIds[i].code; uniqueIds[i].assigned = true; break;

        case 0x18: chFrame.ch_supp[1][0].nodes.x1 = uniqueIds[i].code; chFrame.ch_supp[1][0].nodes.x1done = true;
                   chFrame.ch_supp[1][1].nodes.x1 = uniqueIds[i].code; chFrame.ch_supp[1][1].nodes.x1done = true;
                   uniqueIds[i].assigned = true; break;
        case 0x19: chFrame.ch_supp[1][0].nodes.x2 = uniqueIds[i].code; chFrame.ch_supp[1][0].nodes.x2done = true;
                   chFrame.ch_supp[1][1].nodes.x2 = uniqueIds[i].code; chFrame.ch_supp[1][1].nodes.x2done = true;
                   uniqueIds[i].assigned = true; break;
        case 0x1A: chFrame.ch_supp[1][0].nodes.y1 = uniqueIds[i].code; chFrame.ch_supp[1][0].nodes.y1done = true;
                   chFrame.ch_supp[1][1].nodes.y1 = uniqueIds[i].code; chFrame.ch_supp[1][1].nodes.y1done = true;
                   uniqueIds[i].assigned = true; break;
        case 0x1B: chFrame.ch_supp[1][0].nodes.y2 = uniqueIds[i].code; chFrame.ch_supp[1][0].nodes.y2done = true;
                   chFrame.ch_supp[1][1].nodes.y2 = uniqueIds[i].code; chFrame.ch_supp[1][1].nodes.y2done = true;
                   uniqueIds[i].assigned = true; break;
        case 0x1C: break;
        case 0x1D: break;
        case 0x1E: break;
        case 0x1F: break;
        case 0x20: break;
        case 0x21: break;
        case 0x22: break;
        case 0x23: break;
        case 0x24: break;
        case 0x25: break;
        case 0x26: break;
        case 0x27: break;
        case 0x28: break;
        case 0x29: sentMeta.sensor_id[0] = uniqueIds[i].code; break;
        case 0x2A: sentMeta.sensor_id[1] = uniqueIds[i].code; break;
        case 0x2B: sentMeta.sensor_id[2] = uniqueIds[i].code; break;
        case 0x2C: sentMeta.sensor_id[3] = uniqueIds[i].code; break;
        case 0x2D: break;
        case 0x80: break;
        case 0x98: break;
        default : break;
      }
      if(uniqueIds[i].messageId >= 0x90 && uniqueIds[i].messageId <= 0x90){decodeSentAscii(uniqueIds[i].messageId, uniqueIds[i].code ); uniqueIds[i].assigned = true;}
    }
    bubbleSort(uniqueIds, unique);

    chooseTransferConfig(&chFrame.ch1, "Ch. 1");
    chooseTransferConfig(&chFrame.ch2, "Ch. 2");
    chooseTransferConfig(&chFrame.ch_supp[0][0], "Supplementray Ch.");

    sig.serialStatus = SENT_SER_LOOP;
}

void decodeSentAscii(uint8_t messageId, uint16_t code) {

    uint8_t baseIndex = (messageId - 144) * 2;
    uint8_t firstCharCode = (code >> 6) & 0x3F;
    uint8_t secondCharCode = code & 0x3F;

    char firstChar = firstCharCode + 0x20;
    char secondChar = secondCharCode + 0x20;

    if (baseIndex < 16) { sentMeta.ascii[baseIndex] = firstChar; }
    if (baseIndex + 1 < 16) { sentMeta.ascii[baseIndex + 1] = secondChar;  }
    sentMeta.ascii[16] = '\0';
}

void parseSerialSent(SentSerialMessage msg){
  if(!msg.isValid){return;}
  bool logicalError = false;
  bool newVal = false;
  uint16_t oldData = 0;
  switch(msg.messageId) {
        case 0x00: break;
        case 0x01: if(sentMeta.status != msg.data){sentMeta.status = msg.data; newVal= true; } break;
        case 0x02: break;
        case 0x03: if(sentMeta.chTypes != msg.data){     oldData = sentMeta.chTypes;      sentMeta.chTypes     = msg.data;  logicalError = true; } break;
        case 0x04: if(sentMeta.configCode != msg.data){  oldData = sentMeta.configCode;   sentMeta.configCode  = msg.data;  logicalError = true; } break;
        case 0x05: if(sentMeta.manufacturer != msg.data){oldData = sentMeta.manufacturer; sentMeta.manufacturer= msg.data;  logicalError = true; } break;
        case 0x06: if(sentMeta.sentRev != msg.data){     oldData = sentMeta.sentRev;      sentMeta.sentRev     = msg.data;  logicalError = true; } break;
        case 0x07: if(chFrame.ch1.nodes.x1 != msg.data){ oldData = chFrame.ch1.nodes.x1;  chFrame.ch1.nodes.x1 = msg.data;  logicalError = true; } break;
        case 0x08: if(chFrame.ch1.nodes.x2 != msg.data){ oldData = chFrame.ch1.nodes.x2;  chFrame.ch1.nodes.x2 = msg.data;  logicalError = true; } break;
        case 0x09: if(chFrame.ch1.nodes.y1 != msg.data){ oldData = chFrame.ch1.nodes.y1;  chFrame.ch1.nodes.y1 = msg.data;  logicalError = true; } break;
        case 0x0A: if(chFrame.ch1.nodes.y2 != msg.data){ oldData = chFrame.ch1.nodes.y2;  chFrame.ch1.nodes.y2 = msg.data;  logicalError = true; } break;
        case 0x0B: if(chFrame.ch2.nodes.x1 != msg.data){ oldData = chFrame.ch2.nodes.x1;  chFrame.ch2.nodes.x1 = msg.data;  logicalError = true; } break;
        case 0x0C: if(chFrame.ch2.nodes.x2 != msg.data){ oldData = chFrame.ch2.nodes.x2;  chFrame.ch2.nodes.x2 = msg.data;  logicalError = true; } break;
        case 0x0D: if(chFrame.ch2.nodes.y1 != msg.data){ oldData = chFrame.ch2.nodes.y1;  chFrame.ch2.nodes.y1 = msg.data;  logicalError = true; } break;
        case 0x0E: if(chFrame.ch2.nodes.y2 != msg.data){ oldData = chFrame.ch2.nodes.y2;  chFrame.ch2.nodes.y2 = msg.data;  logicalError = true; } break;
        case 0x0F: break;
        case 0x10: if(chFrame.ch_supp[0][0].raw != msg.data){chFrame.ch_supp[0][0].raw = msg.data; valTransfer(&chFrame.ch_supp[0][0]); newVal =true; } newVal= true; break;
        case 0x11: break;
        case 0x12: if(chFrame.ch_supp[0][0].nodes.x1 != msg.data){oldData = chFrame.ch_supp[0][0].nodes.x1; chFrame.ch_supp[0][0].nodes.x1 = msg.data; logicalError = true; } break;
        case 0x13: if(chFrame.ch_supp[0][0].nodes.x1 != msg.data){oldData = chFrame.ch_supp[0][0].nodes.x2; chFrame.ch_supp[0][0].nodes.x2 = msg.data; logicalError = true; } break;
        case 0x14: if(chFrame.ch_supp[0][0].nodes.x1 != msg.data){oldData = chFrame.ch_supp[0][0].nodes.y1; chFrame.ch_supp[0][0].nodes.y1 = msg.data; logicalError = true; } break;
        case 0x15: if(chFrame.ch_supp[0][0].nodes.x1 != msg.data){oldData = chFrame.ch_supp[0][0].nodes.y2; chFrame.ch_supp[0][0].nodes.y2 = msg.data; logicalError = true; }   break;
        case 0x16: break;
        case 0x17: break;
  }
  if(logicalError){}
  if(newVal && msg.messageId == 0x10){}
  else if(newVal && msg.messageId == 0x01){}
}
