#ifndef SENT_TYPES_H
#define SENT_TYPES_H

#include "config.h"

using BUFF_T = uint32_t;

struct RingBuffer { 
    volatile BUFF_T buffer[RING_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile bool overflow;
};

extern RingBuffer ringBuffer;
extern intr_handle_t sent_isr_handle;
extern portMUX_TYPE ringBufferMux;
extern volatile bool  encBtnFlag;
extern long encRotVal;
extern pcnt_unit_handle_t pcnt_unit;
extern BUFF_T detect[DETECT_BUFFER_SIZE];

struct sortList{ uint16_t idx; BUFF_T val;};

enum outputUpdate { OUT_VCC_OFF, OUT_VCC_ON, OUT_SHORT,OUT_RESET,OUT_NOSIG, OUT_SIG_DETECT, OUT_SIG_INFO, OUT_SERIAL_IDS, OUT_VALUES};
enum serialStatus { SENT_SER_NONE, SENT_SER_COLLECT,SENT_SER_SETUP , SENT_SER_LOOP  };
enum signalStatus { SIG_NONE, SIG_DETECT, SIG_OK };
enum UartStatus   { UART_NONE, UART_SIG_INFO, UART_SENT_SERIAL, UART_VALUES };

extern uint8_t uartState;

struct signalCtrl {
  bool supplyVoltage = AUTO_ENABLE_SENSOR;
  bool overcurrent = false;
  uint32_t ovc_protect_ms = 0;
  uint8_t status = SIG_NONE;
  uint8_t serialStatus = SENT_SER_NONE;
  uint32_t syncLen = 0;
  uint32_t tickLen = 0;
  uint32_t frameLen = 0;
  bool frameLenConst = false;
  uint8_t numPulses = 0;  
  uint8_t dataNibbles = 0;
  const uint32_t absMaxLen = 86400 * ESP_TICKS_PER_US;
  const uint32_t absMinLen = 27 * ESP_TICKS_PER_US;
  uint32_t maxLen = 86400 * ESP_TICKS_PER_US;
  uint32_t minLen = 27 * ESP_TICKS_PER_US;
  uint32_t syncNominal_us = 0;
  uint32_t tickNominal_us = 0;
  uint32_t syncNominal_ticks = 0;
  uint32_t tickNominal_ticks = 0;
  uint32_t lastSyncTime_ms = 0;
  uint32_t lastPulseTime_ms = 0; 
}; 

extern signalCtrl sig;
extern const signalCtrl sigDefault;

struct sentFrame {
    uint32_t syncLen = 0;
    uint8_t status = 0;
    int8_t data[6] = {-1,-1,-1,-1,-1,-1};
    int8_t crc = 0;
    int8_t crcCalc = 0;
    uint32_t frameLen = 0;
    bool crcOk = false;
    bool syncOk = false;
    bool lenOk = false;
    bool validFrame = false;
    bool critFail = false;
};

struct SentSerialMessage {
    uint16_t data = 0;
    uint8_t messageId = 0;
    uint8_t crc = 0;
    bool isEnhanced = false;
    bool config =false;
    bool isValid =false;
};

extern SentSerialMessage lastSentSerial;

struct IDlist{ uint8_t messageId = 0; uint16_t code = 0; uint8_t repeats = 0; bool assigned = false; bool fixedVal = true;};

extern IDlist uniqueIds[MAX_UNIQUE];
extern uint8_t unique;

struct ErrorTracker {
  bool parse = false;
  bool specMissmatch = false;
  bool logicErrorY1data  = false;
  bool logicErrorY2data  = false;
};

extern ErrorTracker error;

struct countStats { 
  int crcOk = 0;
  int crcFail = 0;
  int frames = 0;
  int pulseSkip = 0;
  int pulse_perSec = 0;
  int pulseSkip_percent = 0;
  float frames_perSec = 0;
  float crcFail_percent = 0;
}; 

extern countStats count;

struct metaData {
  int16_t status = -1;       
  int16_t manufacturer = -1; 
  int16_t chTypes = -1; 
  int16_t configCode = -1; 
  int16_t sentRev = -1; 
  char ascii[17] = "";
  int16_t sensor_id[4] ={-1,-1,-1,-1};
};

extern metaData sentMeta;

enum dataTransferTypes {
  ch_undefined,  ch_pressure_lin, ch_temperature_lin_special,  ch_temperature_lin_default, ch_temperature_lin_high, ch_MAF_lin, ch_MAF_sensor_specific,
  ch_position_lin, ch_position_lin_angle, ch_position_sensor_specific, ch_position_multi_dim, ch_ratio_encoding, ch_secure, ch_zero
};

extern const char* chTypeDesc[];

struct nv { 
  uint16_t x1 = 0; 
  bool x1done = false;   
  uint16_t x2 = 0;    
  bool x2done = false;   
  uint16_t y1 = 0;    
  bool y1done = false;   
  uint16_t y2 = 0;    
  bool y2done = false;   
} ;

struct nv_f { 
  float x1 = 0.0;    
  bool x1done = false;  
  float x2 = 0.0;    
  bool x2done = false;   
  float y1 = 0.0;    
  bool y1done = false;   
  float y2 = 0.0;    
  bool y2done = false; 
} ;

enum valueError{ERRVAL_NONE, ERRVAL_INIT, ERRVAL_INVALID, ERRVAL_GENERIC, ERRVAL_FUCTION, ERRVAL_RESERVED, ERRVAL_OEM_DEFINE, ERRVAL_PRODUCTION, ERRVAL_AWAIT_COEFF, ERRVAL_UNEXPECT, ERRVAL_NOIMPLEMENTATION  };
enum valTransferState{VALTRANS_INIT, VALTRANS_NONE,VALTRANS_OK };

struct Channel{
  uint8_t type = ch_undefined;
  uint8_t depth = 0;
  bool statErr  = false;
  uint16_t raw = 0;
  float value = 0.0;
  nv nodes;
  nv_f nodes_f;
  uint8_t valErr = ERRVAL_NONE;
  uint8_t valTrans = VALTRANS_INIT;
  String unit="";
} ;

struct SecureCh {
  uint8_t counter = 0;
  uint8_t missed = 0;
  bool invertMatch = false;
} ;

extern SecureCh secure;

struct ChannelConfig {
  Channel ch1;
  Channel ch2;
  Channel ch_supp[4][2];
  uint8_t nibbles = 0;
} ;

extern ChannelConfig chFrame;
extern const ChannelConfig chDefault;

enum class SENT_ErrorState {
  NONE = -1,
  INIT = 0,
  INVALID = 1,
  FUNC_ERROR = 2,
  GENERIC_ERROR = 3,
  RESERVED_4 = 4,
  RESERVED_3 = 5,
  OEM_DEFINE = 6,
  PRODUCTION = 7,
  AWAIT_COEFF = 8
};

extern SENT_ErrorState currentErrorState;

extern const uint16_t DEFAULT_Y1[];
extern const uint16_t DEFAULT_Y2[];

struct transferCoeff{
  uint8_t N_m_depth;
  bool N_m_signed;
  uint8_t N_e_depth;
  bool N_e_signed; 
  int Xe_offset; 
  char unit[10];
};

extern transferCoeff pressCoeff;
extern transferCoeff tempSpecialCoeff;
extern transferCoeff mafCoeff;
extern transferCoeff positionRelCoeff;
extern transferCoeff positionLinCoeff;
extern transferCoeff positionAngleCoeff;

struct SentErrorStatus { uint16_t code; const char* message; } ;
extern SentErrorStatus sentErrorStatuses[51];

struct SentRevisionCode {
  uint16_t code;
  const char* definition;
  const char* comment;
} ;

extern SentRevisionCode sentRevisionCodes[];

enum tftStates{TFTSTATE_OFF, TFTSTATE_LOGO , TFTSTATE_DASHBOARD, TFTSTATE_MENU, TFTSTATE_PING };
extern uint8_t tftState;

struct encObj {
  bool rotFlag = false;
  bool btnFlag = false;
  long delta = 0;
};

extern encObj enc;
extern int deltaRoll;
extern volatile bool record;

// UART Command System
enum ChannelOutputMode { CH_OFF, CH_RAW, CH_VAL };

struct UartCommandState {
  bool outputActive = false;
  uint32_t outputDelay_ms = 200;
  uint32_t lastOutput_ms = 0;
  bool sendFrameInfo = false;
  bool startRequested = false;
  ChannelOutputMode ch1Mode = CH_OFF;
  ChannelOutputMode ch2Mode = CH_OFF;
  ChannelOutputMode suppMode = CH_OFF;
};

extern UartCommandState uartCmd;

#endif // SENT_TYPES_H
