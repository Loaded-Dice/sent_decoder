#include "sent_types.h"

// Global variable definitions
RingBuffer ringBuffer = {{0}, 0, 0, false};
intr_handle_t sent_isr_handle = NULL;
portMUX_TYPE ringBufferMux = portMUX_INITIALIZER_UNLOCKED;
volatile bool  encBtnFlag = false;
long encRotVal = 0;
pcnt_unit_handle_t pcnt_unit = NULL;
BUFF_T detect[DETECT_BUFFER_SIZE];
uint8_t uartState = UART_NONE;
signalCtrl sig;
const signalCtrl sigDefault;
SentSerialMessage lastSentSerial;
IDlist uniqueIds[MAX_UNIQUE];
uint8_t unique = 0;
ErrorTracker error;
countStats count;
metaData sentMeta;
SecureCh secure;
ChannelConfig chFrame;
const ChannelConfig chDefault;
SENT_ErrorState currentErrorState = SENT_ErrorState::NONE;
encObj enc;
int deltaRoll = 0;
volatile bool record = false;
UartCommandState uartCmd;

const uint32_t baudArr[] = {9600,14400,19200,28800,38400,57600,115200,230400,250000};


const char* chTypeDesc[] = {
"Nicht definiert","Druck, linear","Temperatur, linear (Speziell)","Temperatur, linear (Standard)","Hochemperatur, linear","MAF, linear","MAF (Speziell)",
"Positionssensor, linear","Winkel-Positionssensor, linear","Positionssensor (Speziell)","Positionssensor, mehrdimensional","Relativer Wert","Rolling Code","Nicht belegt",
};

const uint16_t DEFAULT_Y1[] = {3,    6,  12,  24,  48,  96,   193,  385,   770,  1540,  3080};
const uint16_t DEFAULT_Y2[] = {54, 115, 237, 481, 969, 1945, 3896, 7800, 15607, 31221, 62449};

transferCoeff pressCoeff         = {9,  true,3,false,0,"[Pa]\0"};
transferCoeff tempSpecialCoeff   = {11,false,1,true,0,"[K]\0"};
transferCoeff mafCoeff           = {10, true,2,false,0,"[kg/h]\0"};
transferCoeff positionRelCoeff   = {9,  true,3,false,-3,"[%]"};
transferCoeff positionLinCoeff   = {9,  true,3,false,-6,"[m]"};
transferCoeff positionAngleCoeff = {9,  true,3,false,-3,"[Deg]"};

SentErrorStatus sentErrorStatuses[] = {
  {0x000, "No error"},
  {0x001, "Error: Ch. 1 out of range high"},
  {0x002, "Error: Ch. 1 out of range low"},
  {0x003, "Error: Ch. 1 init failed"},
  {0x004, "Error: Ch. 2 out of range high"},
  {0x005, "Error: Ch. 2 out of range low"},
  {0x006, "Error: Ch. 2 init failed"},
  {0x007, "Error: Ch. 1/2 rationality"},
  {0x008, "Reserved"},
  {0x020, "Sensor: Undervoltage"},
  {0x021, "Sensor: Overvoltage"},
  {0x022, "Sensor: Overtemperature"},
  {0x023, "Reserved"},
  {0x030, "Reserved"},
  {0x101, "Error: Mux Ch. 1 high"},
  {0x102, "Error: Mux Ch. 1 low"},
  {0x103, "Error: Mux Ch. 1 init"},
  {0x104, "Error: Mux Ch. 2 high"},
  {0x105, "Error: Mux Ch. 2 low"},
  {0x106, "Error: Mux Ch. 2 init"},
  {0x107, "Error: Mux Ch. 1/2 rationality"},
  {0x108, "Reserved"},
  {0x200, "Reserved"},
  {0x401, "Error: Supp. #1,1 high"},
  {0x402, "Error: Supp. #1,1 low"},
  {0x403, "Error: Supp. #1,1 init"},
  {0x404, "Error: Supp. #2,1 high"},
  {0x405, "Error: Supp. #2,1 low"},
  {0x406, "Error: Supp. #2,1 init"},
  {0x407, "Error: Supp. #3,1 high"},
  {0x408, "Error: Supp. #3,1 low"},
  {0x409, "Error: Supp. #3,1 init"},
  {0x40A, "Error: Supp. #4,1 high"},
  {0x40B, "Error: Supp. #4,1 low"},
  {0x40C, "Error: Supp. #4,1 init"},
  {0x40D, "Error: Supp. #1,2 high"},
  {0x40E, "Error: Supp. #1,2 low"},
  {0x40F, "Error: Supp. #1,2 init"},
  {0x410, "Error: Supp. #2,2 high"},
  {0x411, "Error: Supp. #2,2 low"},
  {0x412, "Error: Supp. #2,2 init"},
  {0x413, "Error: Supp. #3,2 high"},
  {0x414, "Error: Supp. #3,2 low"},
  {0x415, "Error: Supp. #3,2 init"},
  {0x416, "Error: Supp. #4,2 high"},
  {0x417, "Error: Supp. #4,2 low"},
  {0x418, "Error: Supp. #4,2 init"},
  {0x419, "Reserved"},
  {0x420, "Reserved"},
  {0x800, "OEM/Supplier Defined"},
  {0xFFF, "OEM/Supplier Defined"}
};

SentRevisionCode sentRevisionCodes[] = {
  {0x000, "Not specified", ""},
  {0x001, "J2716 FEB2007 SENT Rev. 1","(Only short serial msg)"},
  {0x002, "J2716 FEB2008 SENT Rev. 2","(Only short serial msg)"},
  {0x003, "J2716 JAN2010 SENT Rev. 3","(max 32 serial messages/cycle)"},
  {0x004, "J2716 APR2016 SENT Rev. 4","(max 64 serial messages/cycle)"},
  {0x005, "reserved", ""}
};

uint8_t tftState = TFTSTATE_LOGO;

#ifdef DISPLAY_ACTIVE
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr_valch1 = TFT_eSprite(&tft);
TFT_eSprite spr_valch2 = TFT_eSprite(&tft);
TFT_eSprite spr_valsupp = TFT_eSprite(&tft);
#endif
