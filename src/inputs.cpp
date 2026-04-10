#include "inputs.h"

int16_t get_encoder_value(void) {
    int count = 0;
    ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &count));
    if (count != 0) {
        ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
        ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));
    }
    return (int16_t)count;
}

bool processEncBtn(){
  static bool btnLast = true;
  static unsigned long timer = 0;
  bool btnNew = digitalRead(ENCBTN_PIN);
  if(!btnLast && btnNew && millis() > timer ){ timer = millis() + 50; btnLast = btnNew; return true; }
  btnLast = btnNew;
  return false;
}

void handleInputs(){
  static long encRotValLast = 0;
  static int8_t delta = 0;
  encRotVal += get_encoder_value();
  if(encRotVal > encRotValLast && delta >=0){delta++;}
  if(encRotVal > encRotValLast && delta < 0){delta = 1;}
  if(encRotVal < encRotValLast && delta <=0){delta--;}
  if(encRotVal < encRotValLast && delta > 0){delta = -1;}
  if(delta <= -2 || delta >= 2){
    enc.rotFlag = true;
    enc.delta = (delta <= -2) ? enc.delta -1 : enc.delta +1 ;
    delta = 0; 
    }

  if(encRotVal != encRotValLast){
    encRotValLast = encRotVal;
    }
  if(processEncBtn()){enc.btnFlag = true;}
}

int getEncDelta(){
  if(!enc.rotFlag){enc.delta = 0; return 0;}
  else{int result = enc.delta; enc.delta = 0; enc.rotFlag = false; return result;}
}

bool getEncRot(){return enc.rotFlag;}

bool getEncBtn(){ if(!enc.btnFlag){return false;} else{enc.btnFlag =false; return true;}}
