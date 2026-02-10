#include "info_strings.h"
#include "globals.h"
#include "misc_functions.h"

String getMessageIdDefinition(uint8_t messageId) {
    switch (messageId) {
        case 0x00: return "Not assigned";
        case 0x01: return "Error/Status Codes";
        case 0x02: return "Reserved";
        case 0x03: return "Ch. 1/2 Sensor Type";
        case 0x04: return "Config Code";
        case 0x05: return "Manufacturer Code";
        case 0x06: return "Protocol Revision";
        case 0x07: return "Fast Ch. 1 X1";
        case 0x08: return "Fast Ch. 1 X2";
        case 0x09: return "Fast Ch. 1 Y1";
        case 0x0A: return "Fast Ch. 1 Y2";
        case 0x0B: return "Fast Ch. 2 X1";
        case 0x0C: return "Fast Ch. 2 X2";
        case 0x0D: return "Fast Ch. 2 Y1";
        case 0x0E: return "Fast Ch. 2 Y2";
        case 0x0F: return "Not assigned";
        case 0x10: return "Supp. Ch. #1,1";
        case 0x11: return "Supp. Ch. #1,2";
        case 0x12: return "Supp. Ch. #1 X1";
        case 0x13: return "Supp. Ch. #1 X2";
        case 0x14: return "Supp. Ch. #1 Y1";
        case 0x15: return "Supp. Ch. #1 Y2";
        case 0x16: return "Supp. Ch. #2,1";
        case 0x17: return "Supp. Ch. #2,2";
        case 0x18: return "Supp. Ch. #2 X1";
        case 0x19: return "Supp. Ch. #2 X2";
        case 0x1A: return "Supp. Ch. #2 Y1";
        case 0x1B: return "Supp. Ch. #2 Y2";
        case 0x1C: return "Supp. Ch. #3,1";
        case 0x1D: return "Supp. Ch. #3,2";
        case 0x1E: return "Supp. Ch. #3 X1";
        case 0x1F: return "Reserved";
        case 0x20: return "Supp. Ch. #3 X2";
        case 0x21: return "Supp. Ch. #3 Y1";
        case 0x22: return "Supp. Ch. #3 Y2";
        case 0x23: return "Supp. Ch. #4,1";
        case 0x24: return "Supp. Ch. #4,2";
        case 0x25: return "Supp. Ch. #4 X1";
        case 0x26: return "Supp. Ch. #4 X2";
        case 0x27: return "Supp. Ch. #4 Y1";
        case 0x28: return "Supp. Ch. #4 Y2";
        case 0x29: return "Sensor ID #1";
        case 0x2A: return "Sensor ID #2";
        case 0x2B: return "Sensor ID #3";
        case 0x2C: return "Sensor ID #4";
        case 0x2D: return "Reserved";
        case 0x80: return "OEM/Supplier Defined";
        case 0x90: return "ASCII OEM Codes";
        case 0x98: return "OEM/Supplier Defined";
        default: 
            if (messageId > 0x2D && messageId < 0x80) return "Reserved";
            return "not assigned";
    }
}

String getManufacturerDefinition(uint16_t manufacturerCode) {
    switch (manufacturerCode) {
        case 0x000: return "not specified";
        case 0x001: return "Bosch";
        case 0x002: return "Hitachi";
        case 0x003: return "Continental";
        case 0x004: return "Infineon";
        case 0x005: return "Sensata";
        case 0x006: return "Melexis";
        case 0x007: return "Micronas";
        case 0x008: return "Austria Micro Systems";
        case 0x009: return "Denso";
        case 0x010: return "Bosch";
        case 0x012: return "Stoneridge Inc";
        case 0x020: return "SiemensVDO";
        case 0x032: return "i2s Intelligente Sensorsysteme";
        case 0x040: return "Autoliv";
        case 0x041: return "Autoliv";
        case 0x042: return "Bosch";
        case 0x043: return "Continental";
        case 0x045: return "Elmos";
        case 0x046: return "Freescale";
        case 0x048: return "Hella";
        case 0x049: return "Infineon";
        case 0x04E: return "NXP Semiconductors";
        case 0x04F: return "OnSemi";
        case 0x053: return "ST Microelectronics";
        case 0x054: return "TRW";
        case 0x056: return "Valeo";
        case 0x05A: return "ZMDI";
        case 0x069: return "IHR";
        case 0x073: return "Seskion";
        case 0x080: return "Continental";
        case 0x0FF: return "not specified";
        case 0xFFF: return "not specified";
        default: return "not assigned";
    }
}

String getCodeDefinition(uint8_t messageId, uint16_t code) {
  switch (messageId) {
    case 0x05: return getManufacturerDefinition(code);
    case 0x01: return getErrorDescription(code);
    case 0x06: return getSentRev(code);
    default: return getCodeAsHex(code);
  }
  return getCodeAsHex(code);
}

String getSentRev(uint16_t code){
  String combined = String(sentRevisionCodes[code].definition);
  combined.concat(" ");
  combined.concat(sentRevisionCodes[code].comment);
  return combined;
}

void printAllChInfo(){
  Serial.print("Fast Ch. 1:  ");
  Serial.println(getChInfo(chFrame.ch1));
  Serial.print("Fast Ch. 2:  ");
  Serial.println(getChInfo(chFrame.ch2));
  Serial.print("Supplementary Ch.: ");
  Serial.println(getChInfo(chFrame.ch_supp[0][0]));
}

String getChInfo(Channel ch){
  String info = String(chTypeDesc[ch.type]);

  if(ch.type != ch_undefined){
  info += " ";
  info += String(ch.depth);
  info += " Bit ";   
  if(ch.nodes.x1done || ch.nodes.x2done || ch.nodes.y1done || ch.nodes.y2done){info += "\n (";
    if(ch.nodes.x1done){info += " X1 = ";  info += String(ch.nodes.x1); info += "; ";}
    if(ch.nodes.x2done){info += " X2 = ";  info += String(ch.nodes.x2); info += "; ";}
    if(ch.nodes.y1done){info += " y1 = ";  info += String(ch.nodes.y1); info += "; ";}
    if(ch.nodes.y2done){info += " y2 = ";  info += String(ch.nodes.y2); info += "; ";}
    info += ") ";
    }
  }
  return info;
}

String getErrorDescription(uint16_t code){
  for (uint8_t i = 0; i < SIZE(sentErrorStatuses); i++){if(sentErrorStatuses[i].code == code){ return sentErrorStatuses[i].message;}}
  return "No Description for " + getCodeAsHex(code);
}
