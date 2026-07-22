#include "value_transfer.h"
#include "globals.h"
#include "output.h"
#include <math.h>

void chooseTransferConfig(Channel * ch, String chName){
  transferCoeff tfc;
  bool configXneeded = true;

  // A standard/high temperature channel uses a fixed transfer function and does
  // not need X/Y nodes. If such a channel nonetheless reports X1/X2 and Y1/Y2,
  // switch it to the special (linear) transfer over those nodes when enabled.
  if(special_transf_when_XY_provided
     && (ch->type == ch_temperature_lin_default || ch->type == ch_temperature_lin_high)
     && ch->nodes.x1done && ch->nodes.x2done
     && ch->nodes.y1done && ch->nodes.y2done){
    ch->type = ch_temperature_lin_special;
  }

  if     (ch->type == ch_pressure_lin           ){tfc = pressCoeff;}
  else if(ch->type == ch_temperature_lin_special){tfc = tempSpecialCoeff;}
  else if(ch->type == ch_MAF_lin                ){tfc = mafCoeff;}
  else if(ch->type == ch_ratio_encoding         ){tfc = positionRelCoeff;}
  else if(ch->type == ch_position_lin_angle     ){tfc = positionAngleCoeff;}
  else if(ch->type == ch_position_lin           ){tfc = positionLinCoeff;}
  else{configXneeded = false;}
  String typeDesc = String(chTypeDesc[ch->type]);
  String info = chName + " ( " + typeDesc + " ) ";
  
  if(configXneeded && (!ch->nodes.x1done || !ch->nodes.x2done) ){ errorMsg(info + " config failed: no X1 , X2 provided"); ch->valTrans = VALTRANS_NONE;} 
  else if(!configXneeded && (ch->nodes.x1done || ch->nodes.x2done )){errorMsg(info + " config missmatch: unecessary X1 , X2 provided");}
  
  if(ch->nodes.x1done && ch->nodes.x2done){
    configXLinTransfer(ch, tfc);
    if(ch->nodes_f.x1done && ch->nodes_f.x2done){
       ch->valTrans = VALTRANS_OK; 
       }
    }
    
  if(ch->nodes.x1done && ch->nodes.x2done ){
    configYLinTransfer(ch, !(ch->nodes.y1done && ch->nodes.y2done));
    if(ch->nodes_f.y1done && ch->nodes_f.y2done){
      }
    }

  if(ch->type == ch_temperature_lin_default || ch->type == ch_temperature_lin_high){
    if(ch->nodes.y1done && ch->nodes.y2done){errorMsg(info + " config missmatch: y1,y2 not needed but provided");}
    ch->valTrans = VALTRANS_OK;
  }
  else if(ch->type == ch_zero || ch->type == ch_secure){
    ch->valTrans = VALTRANS_NONE;
  }
  else if(ch->type == ch_position_multi_dim){
    errorMsg(info + " value transfer function not implementet yet");
    ch->valTrans = VALTRANS_NONE;
  }
  else if(ch->type == ch_undefined || ch->type == ch_MAF_sensor_specific || ch->type == ch_position_sensor_specific){
    errorMsg(info + " value transfer not provided in SAE J2716");
    ch->valTrans = VALTRANS_NONE;
  }
  setChUnit(ch);
}

void configXLinTransfer(Channel *ch, transferCoeff tf) {
    if (tf.N_m_depth + tf.N_e_depth != 12) {
        ch->valErr = ERRVAL_INVALID;
        return;
    }

    ch->nodes_f.x1 = calcXlin(tf, ch->nodes.x1);
    ch->nodes_f.x2 = calcXlin(tf, ch->nodes.x2);
    ch->nodes_f.x1done = true;
    ch->nodes_f.x2done = true;
    ch->valErr = ERRVAL_NONE;
}

void configYLinTransfer(Channel *ch, bool useDefaultY) {
    if (ch->depth < 6 || ch->depth > 16) { ch->valErr = ERRVAL_INVALID; return; }
    if(useDefaultY){ 
        if(ch->nodes.y1done && ch->nodes.y1 != DEFAULT_Y1[ch->depth - 6]){ error.logicErrorY1data = true; }
        if(ch->nodes.y2done && ch->nodes.y2 != DEFAULT_Y2[ch->depth - 6]){ error.logicErrorY2data = true; }
        ch->nodes_f.y1 = DEFAULT_Y1[ch->depth - 6]; ch->nodes_f.y1done = true;
        ch->nodes_f.y2 = DEFAULT_Y2[ch->depth - 6]; ch->nodes_f.y2done = true;
    }
    else{
      if(ch->depth < 12){ 
        ch->nodes_f.y1 = ch->nodes.y1 & ((1 << ch->depth) - 1);
        ch->nodes_f.y2 = ch->nodes.y2 & ((1 << ch->depth) - 1);
      }
      else if(ch->depth > 12){
        ch->nodes_f.y1 =  ch->nodes.y1 << (ch->depth - 12);
        ch->nodes_f.y2 =  ch->nodes.y2 << (ch->depth - 12);
        }
      else{
        ch->nodes_f.y1 = ch->nodes.y1;
        ch->nodes_f.y2 = ch->nodes.y2;
        }
    }
    ch->nodes.y1done = true; ch->nodes_f.y1done = true;
    ch->nodes.y2done = true; ch->nodes_f.y2done = true;
    ch->valErr = ERRVAL_NONE;
}

float calcXlin(transferCoeff tfc, uint16_t X_d) {
    uint16_t exponent_mask = (1 << tfc.N_e_depth) - 1;
    uint16_t mantissa_bits_u =  X_d >> (12-tfc.N_m_depth);  
    uint16_t exponent_bits_u = X_d & exponent_mask; 

    int16_t mantissa_bits = tfc.N_m_signed ? signedTwosComplement(mantissa_bits_u, tfc.N_m_depth) : mantissa_bits_u ;
    int16_t exponent_bits = tfc.N_e_signed ? signedTwosComplement(exponent_bits_u, tfc.N_e_depth) : exponent_bits_u ;
    float mantissa = mantissa_bits;
    float exponent = exponent_bits + tfc.Xe_offset;
    
    return mantissa * pow(10, exponent);
}

int16_t signedTwosComplement(uint16_t input, uint8_t bitDepth) {
    bitDepth = CLAMP(bitDepth,1,16);
    uint16_t mask = (1U << bitDepth) - 1;
    uint16_t maskedValue = input & mask;
    uint8_t shiftAmount = 16 - bitDepth;
    return (int16_t)(maskedValue << shiftAmount) >> shiftAmount;
}

void setChUnit(Channel * ch){
  uint8_t t = ch->type;
  if(t == ch_undefined) {ch->unit = "";}
  else if( t == ch_zero) {ch->unit = "[zero]";}
  else if( t == ch_temperature_lin_special ||  t == ch_temperature_lin_default|| t == ch_temperature_lin_high){ch->unit = "[deg C]";}
  else if( t == ch_MAF_lin ||  t == ch_MAF_sensor_specific){ch->unit = "[kg/h]";}
  else if( t == ch_position_sensor_specific ||  t == ch_position_multi_dim){ch->unit = "[??]";}
  else if( t == ch_pressure_lin){ch->unit = "[Pa]";}
  else if( t == ch_position_lin){ch->unit = "[m]";}
  else if( t == ch_position_lin_angle){ch->unit = "[deg]";}
  else if( t == ch_ratio_encoding){ch->unit = "[%]";}
  else if( t == ch_secure){ch->unit = "[secure]";}
}

void valTransfer(Channel * ch) {

  if(ch-> valTrans == VALTRANS_INIT || ch-> valTrans == VALTRANS_NONE || ch->type == ch_secure || ch->type ==  ch_undefined){ return; }

  else if(ch->type == ch_zero && ch->raw != 0x000 ){ ch->valErr = ERRVAL_UNEXPECT ; return;}
  else if(ch->type == ch_temperature_lin_special || ch->type == ch_temperature_lin_default || ch->type == ch_temperature_lin_high){ linearTemperatureTransfer(ch); return;}
  else if(ch->nodes_f.x1done && ch->nodes_f.x2done && ch->nodes_f.y1done && ch->nodes_f.y2done ){ linearTransfer(ch); return;}
  else{ ch->valErr = ERRVAL_NOIMPLEMENTATION; }
  return;
}

uint32_t checkValueError(Channel * ch) {
  uint32_t maxValue = (1 << ch->depth) - 1;
  uint32_t highClamp = maxValue - 7;
  uint32_t sentValue = ch->raw;
  if (sentValue == 0) { ch->valErr = ERRVAL_INIT; return 1;}
  else if (sentValue >= highClamp) {
    if (sentValue > maxValue) { ch->valErr = ERRVAL_PRODUCTION; return highClamp;}
      
    uint8_t errorOffset = sentValue - highClamp;
    switch (errorOffset) {
      case 0: ch->valErr = ERRVAL_INVALID; break;
      case 1: ch->valErr = ERRVAL_FUCTION; break;
      case 2: ch->valErr = ERRVAL_GENERIC; break;
      case 3: ch->valErr = ERRVAL_RESERVED; break;
      case 4: ch->valErr = ERRVAL_RESERVED; break;
      case 5: ch->valErr = ERRVAL_OEM_DEFINE; break;
      case 6: ch->valErr = ERRVAL_PRODUCTION; break;
    }
    return highClamp;
  }
  ch->valErr = ERRVAL_NONE;
  return sentValue;
}

void linearTransfer(Channel * ch) {
  
  uint32_t sentValue = checkValueError(ch);
  if (ch->valErr != ERRVAL_NONE  ) { ch->value =  -999.0;  return ; }
  if (ch->nodes_f.x1done && ch->nodes_f.x2done && ch->nodes_f.y1done && ch->nodes_f.y2done) {
        float slope = (ch->nodes_f.x2 - ch->nodes_f.x1) / (ch->nodes_f.y2 - ch->nodes_f.y1);
        float physicalValue = ch->nodes_f.x1 + slope * ((float)sentValue - ch->nodes_f.y1);
         ch->value = physicalValue; return;
  }
  ch->valErr = ERRVAL_AWAIT_COEFF;
  ch->value =  -999.0 ; return;
}

void linearTemperatureTransfer(Channel* ch) {
    ch->valErr = ERRVAL_NONE;
    uint32_t sentValue = checkValueError(ch);
    if (ch->type == ch_temperature_lin_special) {
        if (ch->nodes_f.x1done && ch->nodes_f.x2done && ch->nodes_f.y1done && ch->nodes_f.y2done) { linearTransfer(ch); return; } 
        else { ch->valErr = ERRVAL_AWAIT_COEFF; ch->value = 0.0;  return; }
    } 
    else {
        if (ch->valErr != ERRVAL_NONE) { ch->value = 0.0; return;  }
        bool useHighTemp = (ch->type == ch_temperature_lin_high);
        bool useDefault = (ch->type == ch_temperature_lin_default);
        if (ch->depth != 8 && ch->depth != 10 && ch->depth != 12) { ch->value = -1.0; return;  }
        uint32_t maxValue = (1 << ch->depth) - 1;
        uint16_t smdVal = sentValue & maxValue;
        uint16_t highClamp = maxValue - 7;
        if (smdVal < 1 || smdVal > highClamp) { ch->value = -2.0; return; }
        float temperature;
        float slope;
        float offset;

        if (useDefault && !useHighTemp) {
            switch (ch->depth) {
                case 12: slope = 8.0; offset = 200.0; break;
                case 10: slope = 4.0; offset = 220.0; break;
                case 8:  slope = 1.0; offset = 220.0; break;
            }
        } 
        else if (useHighTemp && !useDefault) {
            if (ch->depth != 12) { ch->value = -3.0; return;  }
            slope = 3.0;
            offset = 200.0;
        } 
        else { ch->value = 0.0; return;}
        
        temperature = (float)smdVal / slope + offset;
        ch->value = temperature - 273.15; return;
    }
}
