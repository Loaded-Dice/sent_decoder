#include "channel_config.h"
#include "globals.h"
#include "misc_functions.h"

void setChConfig(ChannelConfig * cfg, uint16_t code12bit) {
  String errorMsg = "";  

  // Error checks
  if (code12bit == 0x000 || code12bit > 0xFFF || code12bit == 0x00F || (code12bit >= 0x030 && code12bit <= 0x03F)) { errorMsg = "not specified Sensor Type"; }
  else if ((code12bit >= 0x020 && code12bit <= 0x02F) || code12bit == 0x010) { errorMsg = "reserved for MAF Sensors"; }
  else if (code12bit == 0x040) { errorMsg = "not specified Temperature Sensors"; }
  else if (code12bit == 0x050) { errorMsg = "not specified Position Sensors"; }
  else if ((code12bit >= 0x058 && code12bit <= 0x05F)) { errorMsg = "reserved for Linear Position Sensors"; }
  else if ((code12bit >= 0x066 && code12bit <= 0x06F)) { errorMsg = "reserved for Angle Position Sensors"; }
  else if ((code12bit >= 0x077 && code12bit <= 0x07F)) { errorMsg = "reserved for Relative Position Sensors"; }
  else if ((code12bit >= 0x086 && code12bit <= 0x08F)) { errorMsg = "reserved for Coded Position Sensors"; }
  else if (code12bit == 0x090 || (code12bit >= 0x091 && code12bit <= 0x09F)) { errorMsg = "not specified multi Dimension Position Sensors or reserved"; }
  else if ((code12bit >= 0x0A0 && code12bit <= 0x0AF)) { errorMsg = "reserved for Position Sensors"; }
  else if (code12bit == 0x0B0 || (code12bit >= 0x0B8 && code12bit <= 0x0BF)) { errorMsg = "not specified Ratio Sensors or reserved"; }
  else if (code12bit == 0x0C0 || (code12bit >= 0x0C1 && code12bit <= 0x0CF)) { errorMsg = "not specified Acceleration Sensors or reserved"; }
  else if ((code12bit >= 0x0D0 && code12bit <= 0xFFE)) { errorMsg = "reserved for further Sensor Classes Definitions"; }
  else if (code12bit == 0xFFF) { errorMsg = "not specified Sensor Type"; }

  if (errorMsg != "") {Serial.print("Channel Config Error: "); Serial.print(getCodeAsHex(code12bit));Serial.print(" "); Serial.println(errorMsg); return;}
  uint8_t code = code12bit;

  // Channel assignments per SAE spec
  switch (code) {
    case 0x001: cfg->ch1.type = ch_pressure_lin;             cfg->ch1.depth = 12; cfg->nibbles = 3; break;
    case 0x002: cfg->ch1.type = ch_pressure_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_zero;                         cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x003: cfg->ch1.type = ch_pressure_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_secure;                       cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x004: cfg->ch1.type = ch_pressure_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_secure;                       cfg->ch2.depth = 12; cfg->ch_supp[0][0].type = ch_temperature_lin_default;  cfg->ch_supp[0][0].depth = 12; cfg->nibbles = 6; break;
    case 0x005: cfg->ch1.type = ch_pressure_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_secure;                       cfg->ch2.depth = 12; cfg->ch_supp[0][0].type = ch_temperature_lin_special;  cfg->ch_supp[0][0].depth = 12; cfg->nibbles = 6; break;
    case 0x006: cfg->ch1.type = ch_pressure_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_pressure_lin;                 cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x007: cfg->ch1.type = ch_pressure_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_temperature_lin_default;      cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x008: cfg->ch1.type = ch_pressure_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_temperature_lin_special;      cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x009: cfg->ch1.type = ch_pressure_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_pressure_lin;                 cfg->ch2.depth = 12; cfg->ch_supp[0][0].type = ch_temperature_lin_special; cfg->ch_supp[0][0].depth = 12; cfg->nibbles = 6; break;
    case 0x00B: cfg->ch1.type = ch_pressure_lin;             cfg->ch1.depth = 12; cfg->ch_supp[0][0].type = ch_temperature_lin_special;  cfg->ch_supp[0][0].depth = 12; cfg->nibbles = 3; break;
    case 0x00C: cfg->ch1.type = ch_pressure_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_zero;                         cfg->ch2.depth = 12; cfg->ch_supp[0][0].type = ch_temperature_lin_special; cfg->ch_supp[0][0].depth = 12; cfg->nibbles = 6; break;
    case 0x00D: cfg->ch1.type = ch_pressure_lin;             cfg->ch1.depth = 12; cfg->nibbles = 4; break;
    case 0x00E: cfg->ch1.type = ch_pressure_lin;             cfg->ch1.depth = 12; cfg->ch_supp[0][0].type = ch_temperature_lin_special;  cfg->ch_supp[0][0].depth = 12; cfg->nibbles = 4; break;
    case 0x011: cfg->ch1.type = ch_MAF_lin;                  cfg->ch1.depth = 16; cfg->ch2.type = ch_zero;                         cfg->ch2.depth = 8;  cfg->nibbles = 6; break;
    case 0x012: cfg->ch1.type = ch_MAF_sensor_specific;      cfg->ch1.depth = 16; cfg->ch2.type = ch_zero;                         cfg->ch2.depth = 8;  cfg->nibbles = 6; break;
    case 0x013: cfg->ch1.type = ch_MAF_lin;                  cfg->ch1.depth = 16; cfg->ch2.type = ch_pressure_lin;                 cfg->ch2.depth = 8;  cfg->nibbles = 6; break;
    case 0x014: cfg->ch1.type = ch_MAF_sensor_specific;      cfg->ch1.depth = 16; cfg->ch2.type = ch_pressure_lin;                 cfg->ch2.depth = 8;  cfg->nibbles = 6; break;
    case 0x015: cfg->ch1.type = ch_MAF_lin;                  cfg->ch1.depth = 14; cfg->ch2.type = ch_pressure_lin;                 cfg->ch2.depth = 10; cfg->nibbles = 6; break;
    case 0x016: cfg->ch1.type = ch_MAF_sensor_specific;      cfg->ch1.depth = 14; cfg->ch2.type = ch_pressure_lin;                 cfg->ch2.depth = 10; cfg->nibbles = 6; break;
    case 0x017: cfg->ch1.type = ch_MAF_lin;                  cfg->ch1.depth = 14; cfg->ch2.type = ch_zero;                         cfg->ch2.depth = 10; cfg->nibbles = 6; break;
    case 0x018: cfg->ch1.type = ch_MAF_sensor_specific;      cfg->ch1.depth = 14; cfg->ch2.type = ch_zero;                         cfg->ch2.depth = 10; cfg->nibbles = 6; break;
    case 0x019: cfg->ch1.type = ch_MAF_lin;                  cfg->ch1.depth = 14; cfg->ch2.type = ch_temperature_lin_default;      cfg->ch2.depth = 10; cfg->nibbles = 6; break;
    case 0x01A: cfg->ch1.type = ch_MAF_sensor_specific;      cfg->ch1.depth = 14; cfg->ch2.type = ch_temperature_lin_default;      cfg->ch2.depth = 10; cfg->nibbles = 6; break;
    case 0x01B: cfg->ch1.type = ch_MAF_lin;                  cfg->ch1.depth = 16; cfg->ch2.type = ch_temperature_lin_default;      cfg->ch2.depth = 8;  cfg->nibbles = 6; break;
    case 0x01C: cfg->ch1.type = ch_MAF_sensor_specific;      cfg->ch1.depth = 16; cfg->ch2.type = ch_temperature_lin_default;      cfg->ch2.depth = 8;  cfg->nibbles = 6; break;
    case 0x01D: cfg->ch1.type = ch_MAF_sensor_specific;      cfg->ch1.depth = 12; cfg->ch2.type = ch_zero;                         cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x01E: cfg->ch1.type = ch_MAF_sensor_specific;      cfg->ch1.depth = 12; cfg->ch2.type = ch_pressure_lin;                 cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x01F: cfg->ch1.type = ch_MAF_sensor_specific;      cfg->ch1.depth = 12; cfg->ch2.type = ch_temperature_lin_default;      cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x041: cfg->ch1.type = ch_temperature_lin_default;  cfg->ch1.depth = 12; cfg->ch2.type = ch_zero;                         cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x042: cfg->ch1.type = ch_temperature_lin_default;  cfg->ch1.depth = 12; cfg->ch2.type = ch_secure;                       cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x043: cfg->ch1.type = ch_temperature_lin_default;  cfg->ch1.depth = 12; cfg->ch2.type = ch_temperature_lin_default;      cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x044: cfg->ch1.type = ch_temperature_lin_default;  cfg->ch1.depth = 12; cfg->nibbles = 6; break;
    case 0x045: cfg->ch1.type = ch_temperature_lin_high;     cfg->ch1.depth = 12; cfg->ch2.type = ch_zero;                         cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x046: cfg->ch1.type = ch_temperature_lin_high;     cfg->ch1.depth = 12; cfg->ch2.type = ch_secure;                       cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x047: cfg->ch1.type = ch_temperature_lin_high;     cfg->ch1.depth = 12; cfg->ch2.type = ch_temperature_lin_high;         cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x048: cfg->ch1.type = ch_temperature_lin_high;     cfg->ch1.depth = 12; cfg->nibbles = 6; break;
    case 0x049: cfg->ch1.type = ch_temperature_lin_special;  cfg->ch1.depth = 12; cfg->ch2.type = ch_zero;                         cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x04A: cfg->ch1.type = ch_temperature_lin_special;  cfg->ch1.depth = 12; cfg->ch2.type = ch_secure;                       cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x04B: cfg->ch1.type = ch_temperature_lin_special;  cfg->ch1.depth = 12; cfg->ch2.type = ch_temperature_lin_special;      cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x04C: cfg->ch1.type = ch_temperature_lin_special;  cfg->ch1.depth = 12; cfg->nibbles = 6; break;
    case 0x051: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->nibbles = 3; break;
    case 0x052: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_zero;                         cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x053: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->nibbles = 4; break;
    case 0x054: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_position_lin;                 cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x055: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_secure;                       cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x056: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_temperature_lin_default;      cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x057: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_ratio_encoding;               cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x060: cfg->ch1.type = ch_position_lin_angle;       cfg->ch1.depth = 12; cfg->nibbles = 3; break;
    case 0x061: cfg->ch1.type = ch_position_lin_angle;       cfg->ch1.depth = 12; cfg->ch2.type = ch_zero;    cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x062: cfg->ch1.type = ch_position_lin_angle;       cfg->ch1.depth = 12; cfg->nibbles = 4; break;
    case 0x063: cfg->ch1.type = ch_position_lin_angle;       cfg->ch1.depth = 12; cfg->ch2.type = ch_position_lin_angle;   cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x064: cfg->ch1.type = ch_position_lin_angle;       cfg->ch1.depth = 12; cfg->ch2.type = ch_secure;  cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x065: cfg->ch1.type = ch_position_lin_angle;       cfg->ch1.depth = 12; cfg->ch2.type = ch_temperature_lin_default; cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x070: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->nibbles = 3; break;
    case 0x071: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_zero;    cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x072: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->nibbles = 4; break;
    case 0x073: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_position_lin;         cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x074: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_secure;  cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x075: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_temperature_lin_default; cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x076: cfg->ch1.type = ch_position_lin;             cfg->ch1.depth = 12; cfg->ch2.type = ch_ratio_encoding;    cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x080: cfg->ch1.type = ch_position_sensor_specific; cfg->ch1.depth = 12; cfg->nibbles = 3; break;
    case 0x081: cfg->ch1.type = ch_position_sensor_specific; cfg->ch1.depth = 12; cfg->ch2.type = ch_zero;    cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x082: cfg->ch1.type = ch_position_sensor_specific; cfg->ch1.depth = 12; cfg->nibbles = 4; break;
    case 0x083: cfg->ch1.type = ch_position_sensor_specific; cfg->ch1.depth = 12; cfg->ch2.type = ch_position_sensor_specific; cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x084: cfg->ch1.type = ch_position_sensor_specific; cfg->ch1.depth = 12; cfg->ch2.type = ch_secure;  cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x085: cfg->ch1.type = ch_position_sensor_specific; cfg->ch1.depth = 12; cfg->ch2.type = ch_temperature_lin_default; cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x0B1: cfg->ch1.type = ch_ratio_encoding;           cfg->ch1.depth = 12; cfg->nibbles = 3; break;
    case 0x0B2: cfg->ch1.type = ch_ratio_encoding;           cfg->ch1.depth = 12; cfg->ch2.type = ch_zero;    cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x0B3: cfg->ch1.type = ch_ratio_encoding;           cfg->ch1.depth = 12; cfg->nibbles = 4; break;
    case 0x0B4: cfg->ch1.type = ch_ratio_encoding;           cfg->ch1.depth = 12; cfg->ch2.type = ch_ratio_encoding;    cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x0B5: cfg->ch1.type = ch_ratio_encoding;           cfg->ch1.depth = 12; cfg->ch2.type = ch_secure;  cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x0B6: cfg->ch1.type = ch_ratio_encoding;           cfg->ch1.depth = 12; cfg->ch2.type = ch_temperature_lin_default; cfg->ch2.depth = 12; cfg->nibbles = 6; break;
    case 0x0B7: cfg->ch1.type = ch_ratio_encoding;           cfg->ch1.depth = 12; cfg->ch2.type = ch_pressure_lin;      cfg->ch2.depth = 12; cfg->nibbles = 6; break;
  }
}
