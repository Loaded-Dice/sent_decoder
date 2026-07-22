#include "display.h"

#ifdef DISPLAY_ACTIVE

#include "globals.h"
#include "inputs.h"
#include "misc_functions.h"
#include "misc_functions.h"
#include "img.h"

// Forward declarations for internal functions
void drawArrow(int16_t x, int16_t y, uint32_t color);
void set_img();
void set_img(bool init);
void drawValues();
const String chValString(const Channel& ch);
uint16_t gray(uint8_t gscale);
void drawMonochromeBitmapAs(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, int colorWhite, int colorBlack);
void drawFrameDiff(int16_t x, int16_t y, const uint8_t* prev, const uint8_t* curr, int16_t w, int16_t h, uint16_t mask1 = TFT_WHITE, uint16_t mask0 = TFT_BLACK);

static uint8_t pingReturnState = TFTSTATE_MENU;
static uint32_t pingScreenEnd_ms = 0;

void triggerPingScreen(uint32_t durationMs) {
  if (tftState != TFTSTATE_PING) {
    pingReturnState = tftState;
  }
  pingScreenEnd_ms = millis() + durationMs;
  tftState = TFTSTATE_PING;
}

void displayIdentifier() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM); // Middle Center
  
  // Display is 160x128, try largest possible size
  String identifier = IDENTIFY;
  
  // Test different text sizes to find best fit
  int textSize = 4;
  tft.setTextSize(textSize);
  int textWidth = tft.textWidth(identifier);
  
  // Reduce size if text doesn't fit (with 10px margin on each side)
  while (textWidth > 140 && textSize > 1) {
    textSize--;
    tft.setTextSize(textSize);
    textWidth = tft.textWidth(identifier);
  }
  
  // Draw centered text
  tft.drawString(identifier, 80, 64);
  
  // Show for 3 seconds
  delay(3000);
  tft.fillScreen(TFT_BLACK);
}

void chkTft(){
}

int dashSel = 0;


void tftMain(){
  static uint8_t tftStateCurrent = TFTSTATE_OFF;
  static unsigned long tftTimeout = 0;
  
  if(millis() < tftTimeout) { return; }

  bool initNewScreen = false;
  
  if(tftStateCurrent != tftState){ tft.fillScreen(TFT_BLACK); initNewScreen = true; tftStateCurrent = tftState;}
  
  if(tftState == TFTSTATE_LOGO){drawMonochromeBitmapAs(0, 0, img_SENT_Logo, 160, 128, TFT_WHITE,TFT_BLACK); tftTimeout = millis() + 1500; tftState = TFTSTATE_MENU; return; }
  else if(tftState == TFTSTATE_PING){
    if(initNewScreen){
      tft.fillScreen(TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextSize(5);
      tft.drawString("PING", 80, 64);
      tft.drawString("PING", 81, 64);
      tft.drawString("PING", 80, 65);
      tft.drawString("PING", 81, 65);
    }
    if((int32_t)(millis() - pingScreenEnd_ms) >= 0){
      tftState = pingReturnState;
    }
    return;
  }
  else if(tftState == TFTSTATE_MENU){
    EVERY_N_MILLIS(50){ drawMenu(initNewScreen);}      
  }
  else if(tftState == TFTSTATE_DASHBOARD){

     if(getEncRot()){dashSel += getEncDelta();
    }
    EVERY_N_MILLIS(200) { set_img(initNewScreen); }
    EVERY_N_MILLIS(100){drawValues();}
    }
}

// --------------===================={MAIN MENU}====================-------------- //

void setMenuPos(uint8_t * selectPos, uint8_t * lineOff,  int8_t posDelta, uint8_t arraySize, uint8_t maxLines){
  int8_t newPos = (int8_t)(*selectPos) + posDelta; 
  int8_t newLine = (int8_t)(*lineOff);
  if(newPos < 0 ){newLine--; newPos = 0;}
  else if(newPos > maxLines-1){newLine++; newPos=maxLines-1;}
  if(newLine> arraySize - maxLines){newLine = arraySize - maxLines;}
  else if(newLine < 0){newLine=0;}
  *selectPos = (uint8_t)newPos; *lineOff = (uint8_t)newLine;
 // Serial.print("newPos: ");Serial.print(newPos);Serial.print('\t');Serial.print("NewLine: ");Serial.println(newLine);
}

const String menuEntries[] ={"5V Vcc","Dashboard","Sig.Reset", "Sig.Status","Sig.Serial","UART COM"};

uint16_t getEntryColor(uint8_t idx){
  switch(idx){
    case 0 :  return sig.supplyVoltage ? TFT_GREEN : TFT_RED;
    case 1 :  return sig.supplyVoltage ? TFT_WHITE : TFT_DARKGREY;
    case 2 :  return sig.supplyVoltage ? TFT_WHITE : TFT_DARKGREY;
    case 3 :  return sig.supplyVoltage ? TFT_WHITE : TFT_DARKGREY;
    case 4 :  return sig.supplyVoltage ? TFT_WHITE : TFT_DARKGREY;
    case 5 :  return TFT_WHITE;
    default : return TFT_WHITE;
  }
  return TFT_WHITE;
}

String getSignalStatus(){
       if(sig.status == SIG_NONE){return "None";}
  else if(sig.status == SIG_DETECT){return "Analyze";}
  else if(sig.status == SIG_OK){return "OK";}
  return "";
}

String getSerialStatus(){
       if(sig.serialStatus == SENT_SER_NONE){return "None";}
  else if(sig.serialStatus == SENT_SER_COLLECT){return "Collect";}
  else if(sig.serialStatus == SENT_SER_SETUP){return "Analyze";}
  else if(sig.serialStatus == SENT_SER_LOOP){return "OK";}
  return "";
}

String getEntryValue(uint8_t idx){
  switch(idx){
    case 0 :  return sig.supplyVoltage ? " On" : "Off";
    case 1 :  return "";
    case 2 :  return "";
    case 3 :  return  getSignalStatus();
    case 4 :  return  getSerialStatus();
    case 5 :  return ((Serial) ? "Active" : "Inaktive");
    default : return "";
  }
  return "";
}

void menuBtnHandler(uint8_t idx){
  bool vcc = sig.supplyVoltage;
  switch(idx){
    case 0 :  vcc ? resetSensor() : startSensor(); tftState = TFTSTATE_DASHBOARD; break;
      // if(vcc){resetSensor();} 
      // else{startSensor(); tftState = TFTSTATE_DASHBOARD; }  
      // break;
    case 1 :  vcc ? tftState = TFTSTATE_DASHBOARD : tftState = TFTSTATE_MENU; break ;
    case 2 :  resetSensor(); if(vcc){ startSensor(); tftState = TFTSTATE_DASHBOARD ;}  break ;
    case 3 :  break ;
    case 4 :  break ;
    case 5 :  break ;
    default : break ;
  }
}

void drawMenu(){drawMenu(false);}

void drawMenu(bool init){

  const uint8_t  menuSize = SIZE(menuEntries);
  const uint8_t  maxLines = menuSize < 8 ? menuSize : 8;
  static uint8_t inPosSel = 0;
  static uint8_t inPosSelLast = 1;
  static uint8_t inLineOff = 0;
  static uint8_t inLineOffLast = 1; 

  const uint8_t xOffMenuTxt = 25;
  const uint8_t xOffMenuVal = xOffMenuTxt + 70;
  const uint8_t yOffMenuTxt = 35;
  const uint8_t yGapMenuTxt = 2;
  const uint8_t xOffMenuSel = 5;

  static uint8_t sigLast = SIG_NONE;
  static uint8_t serialLast = SENT_SER_NONE;
  static bool uartLast = false;
  static bool sigVccLast = false;

  bool updateEnries = (sig.status != sigLast || serialLast != sig.serialStatus || uartLast != Serial || sigVccLast != sig.supplyVoltage);
  sigLast = sig.status;
  serialLast = sig.serialStatus;
  sigVccLast = sig.supplyVoltage;
  uartLast = Serial;
  
  static uint16_t arrowPos[2] = {0,0};
  static uint16_t arrowPosLast[2] = {0,0};
  tft.setTextDatum(TL_DATUM);
  bool btn = getEncBtn();
  bool rot = getEncRot();

   if(btn){  menuBtnHandler(inPosSel);  }
   if(rot){ setMenuPos(&inPosSel, &inLineOff, getEncDelta() , menuSize , maxLines); }

    arrowPos[0] = xOffMenuSel;
    arrowPos[1] = yOffMenuTxt + inPosSel * (yGapMenuTxt + tft.fontHeight()) ;
    if( arrowPosLast[0] != arrowPos[0] || arrowPosLast[1] != arrowPos[1] || init){
      drawArrow(arrowPosLast[0],  arrowPosLast[1],TFT_BLACK);
      drawArrow(arrowPos[0],  arrowPos[1],TFT_WHITE);
      arrowPosLast[0] = arrowPos[0]; arrowPosLast[1] = arrowPos[1];
    }

  if(inLineOff != inLineOffLast || updateEnries || init){
    for ( int8_t i = 0; i < (( menuSize > maxLines) ? maxLines : menuSize) ; i++){
        uint8_t idx = i + inLineOff;
        tft.setTextSize(1);
        tft.setTextColor(getEntryColor(idx),TFT_BLACK,true);
        uint16_t yOff = yOffMenuTxt + i * (yGapMenuTxt + tft.fontHeight());
        tft.drawString(menuEntries[idx], xOffMenuTxt, yOff );
        if(getEntryValue(idx) != ""){ tft.drawString(getEntryValue(idx), xOffMenuVal, yOff); }
    }
  }
  inPosSelLast = inPosSel;  
  inLineOffLast = inLineOff;    
}

void drawArrow(int16_t x,int16_t y, uint32_t color){ tft.drawBitmap(  x,  y, img_arrow_7x7, 7, 7,color); }

// --------------===================={DASHBOARD}====================-------------- //
void set_img(){set_img(false);}

void set_img(bool init){
  //dispYoff
  
 
  const uint16_t colorImpFrame = TFT_WHITE;
  const uint16_t colorImpDiag = TFT_WHITE;
  const uint16_t colorParseCh  = TFT_WHITE;
  const uint16_t colorBG = TFT_BLACK;

  static uint8_t sigStatusLast = SIG_NONE;
  static uint8_t serialStatusLast = SENT_SER_NONE;
  if(sigStatusLast == sig.status && serialStatusLast == sig.serialStatus && !init) return;
  
  sigStatusLast = sig.status;
  serialStatusLast = sig.serialStatus;
  static const unsigned char  *impulsePerFrame_last = nullptr;
  static const unsigned char  *imulseDiag_last      = nullptr;
  static const unsigned char  *parseToCh_last       = nullptr;
   const unsigned char  *impulsePerFrame = nullptr;
   const unsigned char  *imulseDiag      = nullptr;
   const unsigned char  *parseToCh       = nullptr;

    if(sig.status == SIG_NONE){         impulsePerFrame = img_0_impulse;    imulseDiag = img_imp_no_signal;   }
    else if(sig.status == SIG_DETECT){  imulseDiag = img_imp_detect;        impulsePerFrame = img_0_impulse;  }

    else if(sig.status == SIG_OK){ 

           if(chFrame.nibbles == 3 &&  sig.frameLenConst){imulseDiag = img_imp_N3_CRC_P;}
      else if(chFrame.nibbles == 3 && !sig.frameLenConst){imulseDiag = img_imp_N3_CRC;}
      else if(chFrame.nibbles == 4 &&  sig.frameLenConst){imulseDiag = img_imp_N4_CRC_P;}
      else if(chFrame.nibbles == 4 && !sig.frameLenConst){imulseDiag = img_imp_N4_CRC;}
      else if(chFrame.nibbles == 5 &&  sig.frameLenConst){imulseDiag = img_imp_N5_CRC_P;}
      else if(chFrame.nibbles == 5 && !sig.frameLenConst){imulseDiag = img_imp_N5_CRC;}
      else if(chFrame.nibbles == 6 &&  sig.frameLenConst){imulseDiag = img_imp_N6_CRC_P;}
      else if(chFrame.nibbles == 6 && !sig.frameLenConst){imulseDiag = img_imp_N6_CRC;}
      else{ imulseDiag = img_imp_detect;}

      switch(sig.numPulses){
        case  0 :     impulsePerFrame = img_0_impulse; break;
        case  6 :     impulsePerFrame = img_6_impulse; break;
        case  7 :     impulsePerFrame = img_7_impulse; break;
        case  8 :     impulsePerFrame = img_8_impulse; break;
        case  9 :     impulsePerFrame = img_8_impulse; break;
        case 10 :     impulsePerFrame = img_10_impulse; break;
        default :     impulsePerFrame = nullptr;  break;
      }  
      tft.setTextSize(1);
      tft.setTextColor(TFT_WHITE);
      tft.setCursor(0, 0);
    }
    if(sig.serialStatus == SENT_SER_LOOP){ 
           if(chFrame.ch1.depth == 12 && chFrame.ch2.depth == 12 && chFrame.nibbles == 6 ){parseToCh = img_Ch1_12Bit_Ch2_12Bit;}
      else if(chFrame.ch1.depth == 12 && chFrame.nibbles == 4 && chFrame.ch2.type == ch_undefined){parseToCh =  img_Ch1_12Bit_Fast;}
      else if(chFrame.ch1.depth == 14 && chFrame.ch2.depth == 10 && chFrame.nibbles == 6 ){ parseToCh =  img_Ch1_14Bit_Ch2_10Bit;}
      else if(chFrame.ch1.depth == 16 && chFrame.ch2.depth == 8  && chFrame.nibbles == 6 ){ parseToCh = img_Ch1_16Bit_Ch2_8Bit;}
    }
    else{ parseToCh = nullptr;}

    if(impulsePerFrame_last != impulsePerFrame || init){    
      impulsePerFrame_last = impulsePerFrame;
      if(impulsePerFrame  == nullptr){tft.fillRect(1,  1,  160,  12, colorBG);}
      else{drawMonochromeBitmapAs(1, 1,  impulsePerFrame, 160, 12, colorImpFrame , colorBG);
        if(sig.syncNominal_us != 0){
            tft.setTextSize(1); 
            tft.setTextColor(TFT_WHITE,TFT_BLACK,true);
            tft.drawString(String(sig.syncNominal_us),11,48);
            tft.drawString("[uS]",8,56);
          }
      }
    }
  
    if(imulseDiag_last != imulseDiag || init){
      imulseDiag_last = imulseDiag;
      if(imulseDiag == nullptr){tft.fillRect(0,  13,  160,  29, colorBG);}
      else{drawMonochromeBitmapAs(0, 13, imulseDiag, 160, 29, colorImpDiag, colorBG);}
    }

    if(parseToCh_last != parseToCh || init){
      parseToCh_last = parseToCh;
      if(parseToCh == nullptr){tft.fillRect(48, 45,  88,  18, colorBG);}
      else{drawMonochromeBitmapAs(48, 45, parseToCh, 88, 18, colorParseCh, colorBG);}
    }
}

void drawValues(){
  const int16_t xVal = 13;
  uint16_t yLine = 75;
  const uint16_t yDelta = 10;

if(getEncBtn()){tftState = TFTSTATE_MENU;}  
  String buff = "";
  static float crc_tft = 0.0;
  static int fps_tft = 0;
  if(sig.serialStatus != SENT_SER_LOOP){return;}

  uint8_t ch1_x = 0;
  uint8_t ch2_x = 0;

  if(chFrame.ch1.depth == 12 && chFrame.ch2.depth == 12 && chFrame.nibbles == 6 ){ch1_x = 18; ch2_x = 57;}
  else if(chFrame.ch1.depth == 12 && chFrame.nibbles == 4 && chFrame.ch2.type == ch_undefined){ch1_x = 21; ch2_x = 0;}
  else if(chFrame.ch1.depth == 14 && chFrame.ch2.depth == 10 && chFrame.nibbles == 6 ){ ch1_x = 22; ch2_x = 61 ;}
  else if(chFrame.ch1.depth == 16 && chFrame.ch2.depth == 8  && chFrame.nibbles == 6 ){ ch1_x = 25; ch2_x = 8;}
    
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE,TFT_BLACK,true);
    buff = "Ch1:  ";    buff += chValString(chFrame.ch1); buff+="               "; tft.drawString(buff.substring(0,21),xVal,yLine);    yLine += yDelta;
    buff = "Ch2:  ";    buff += chValString(chFrame.ch2); buff+="               "; tft.drawString(buff.substring(0,21),xVal,yLine);    yLine += yDelta;
    buff = "Supp: ";    buff += chValString( chFrame.ch_supp[0][0]); buff+="               "; tft.drawString(buff.substring(0,21),xVal,yLine);    yLine += yDelta;
    // 22 = max len  22 - strlen(buff)
   // strncat ( buff, "               ", CLAMP(22 - buff.length(),0,22) );
    if(fps_tft != count.frames_perSec){
      fps_tft = count.frames_perSec;
      buff = "SENT FPS: ";      buff += String(fps_tft); buff+="               "; tft.drawString(buff.substring(0,21),xVal,yLine);      yLine += yDelta;
    }
    float crc_neu = (float)(10000 - (int)(count.crcFail_percent * 100)) / 100.0 ;
    if(crc_tft != crc_neu){
      crc_tft = crc_neu;
      buff = "CRC ok[%]: ";      buff += String(crc_neu);  buff+="               "; tft.drawString(buff.substring(0,21),xVal,yLine);      yLine += yDelta;
    }
}

const String chValString(const Channel& ch) {
  String result;
    if(ch.type == ch_zero || ch.type == ch_position_sensor_specific || ch.type == ch_position_multi_dim || ch.valTrans == VALTRANS_NONE){ result = getValAs0xHex(ch.raw, ch.depth) + String(" Raw"); }
    else{   result = String(ch.value) + String(" ") + String(ch.unit); }
    return result;
}

// --------------===================={MISC}====================-------------- //

uint16_t gray(uint8_t gscale){ return tft.color565(gscale,gscale, gscale);}

void drawMonochromeBitmapAs(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, int colorWhite, int colorBlack) {
  //y-= dispYoff;
  for (int16_t j = 0; j < h; j++) {
    for (int16_t i = 0; i < w; i += 8) {
      uint8_t byte = bitmap[j * (w / 8) + (i / 8)];
      for (uint8_t n = 0; n < 8 && (i + n) < w; n++) {
        bool pixel = ((byte >> (7 - n)) & 1);
        if (pixel == 0 && colorBlack != TFT_TRANSPARENT) { tft.drawPixel(x + i + n, y + j, (uint16_t)colorBlack); } 
        else if (pixel == 1 && colorWhite != TFT_TRANSPARENT) { tft.drawPixel(x + i + n, y + j, (uint16_t)colorWhite); }
      }
    }
  }
}

void drawFrameDiff(int16_t x, int16_t y, const uint8_t* prev, const uint8_t* curr, int16_t w, int16_t h, uint16_t mask1, uint16_t mask0){

  if (prev == nullptr && curr == nullptr) return;

  for (int i = 0; i < (w * h / 8); ++i) {
    uint8_t oldByte = prev ? pgm_read_byte(&prev[i]) : 0x00;
    uint8_t newByte = curr ? pgm_read_byte(&curr[i]) : 0x00;
    uint8_t diff = oldByte ^ newByte;
    if (diff == 0) continue;

    int byteIndex = i;
    int baseX = (byteIndex % (w / 8)) * 8;
    int y1 = byteIndex / (w / 8);

    for (int bit = 0; bit < 8; ++bit) {
      if (diff & (1 << bit)) {
        int x1 = baseX + bit;
        bool pixelSet = (newByte >> bit) & 1;
        uint16_t col = pixelSet ? mask1 : mask0;
        if (col >= 0) tft.drawPixel(x1 + x, y1 + y, col);
      }
    }
  }
}

#endif // DISPLAY_ACTIVE
