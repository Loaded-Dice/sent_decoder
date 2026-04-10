#include "periodic_tasks.h"
#include "globals.h"
#include "misc_functions.h"
#include "output.h"
#include "display.h"
#include "uart_commands.h"
#include "json_output.h"

bool acknowledge = false;

void serialRead() {

  static String inputString = "";
  if (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') { serialHandler(inputString); inputString = ""; }
    else{inputString += inChar;}
  }
}

uint32_t reenableDelay_ms = 15000;

void serialHandler(String msg){
  msg.trim();
  acknowledge = true;
  
  // Check if this is a new command system command (starts with -)
  if (msg.startsWith("-")) {
    parseAndExecuteCommand(msg);
    return;
  }
  
  // Legacy command system
  msg.toUpperCase();
  String answer = "";
   if(msg.startsWith("START")){
     if(sig.supplyVoltage){startSensor(); answer = "";}
     
     }
  else if(msg.startsWith("STOP")){resetSensor();  answer = "Stoppe Sent Sensor!"; }
  else if(msg.startsWith("RESET")){resetSensor(); answer = "Reset Sent Sensor!"; }
  else if(msg.startsWith("DETECT")){detectSignal(); answer = "Detect Signal";}  
  
  #ifdef DISPLAY_ACTIVE
  else if(msg.startsWith("TFT")){set_img();} 
  #endif
  
  else{answer =  "Unbekannter befehl: " + msg;}
  if(answer != ""){ infoMsg(answer); }
}

void periodicUART(){
//EVERY_N_MILLIS(500){  Serial.print(getSerialStatus());Serial.print("\t"); Serial.println( getSignalStatus()); return;}

  sendContinuousOutput();
  
  // Check if frame info should be sent
  if (uartCmd.sendFrameInfo && sig.serialStatus == SENT_SER_LOOP && sig.status == SIG_OK) {
    sendCompleteInfo();
    uartCmd.sendFrameInfo = false;
  }
  
  // Check if start was requested and signal is ready
  if (uartCmd.startRequested && !uartCmd.outputActive && sig.serialStatus == SENT_SER_LOOP && sig.status == SIG_OK) {
    if (uartCmd.ch1Mode == CH_OFF && uartCmd.ch2Mode == CH_OFF && uartCmd.suppMode == CH_OFF) {
      uartCmd.startRequested = false;
    } 
    else {
      uartCmd.outputActive = true;
      uartCmd.lastOutput_ms = millis();
      uartCmd.startRequested = false;
      infoMsgJson("Kontinuierliche Ausgabe gestartet (Intervall: " + String(uartCmd.outputDelay_ms) + "ms)");
    }
  }

  static uint8_t sigLast = SIG_NONE;
  static uint8_t serialLast = SENT_SER_NONE;

    
  if(!acknowledge){
    EVERY_N_MILLIS(500){infoMsg("Warte auf Kommandoeingabe... sende -help zum anzeigen der Befehle."); return;  }
  }
  bool infoUpdate = (sig.status != sigLast || serialLast != sig.serialStatus);
  sigLast = sig.status;
  serialLast = sig.serialStatus;
  if(infoUpdate){ infoMsg(getSignalStatusMsg()); }

}

String getSignalStatusMsg(){
    if(sig.status == SIG_NONE){ return "kein Signal"; }
    else if(sig.status == SIG_DETECT){ return "Analysiere Signal"; }
    else if(sig.status == SIG_OK){ 
         if( sig.serialStatus == SENT_SER_NONE){ return "Signalanalyse abgeschlossen. Warte auf Slow Channel";}
   else  if( sig.serialStatus == SENT_SER_COLLECT){ return "Sammle mehrere serielle Sent Datenzyklen ";}
   else  if( sig.serialStatus == SENT_SER_SETUP){ return "parse serielle Sent Daten";}
   else  if( sig.serialStatus == SENT_SER_LOOP){ return "Daten parsing abgeschlossen"; }
    }
  return "";
}

void calcStats(){
  EVERY_N_MILLIS(1000){
    bool ok = (sig.status == SIG_OK);
    count.frames_perSec = !ok ? 0 : count.frames;
    if(count.frames_perSec != 0){ count.crcFail_percent = (float)count.crcFail * 100.0 / (float)count.frames_perSec; }
    else{count.crcFail_percent = 0;}
    count.pulse_perSec = !ok ? 0 : count.frames_perSec * sig.numPulses;
    if(count.pulse_perSec != 0){count.pulseSkip_percent = count.pulseSkip * 100 / count.pulse_perSec;}
    else{count.pulseSkip_percent = 0;}
    if(count.crcFail > count.crcOk){sig.status = SIG_DETECT; clearRingBuff();}
    count.crcOk = 0;
    count.crcFail = 0;
    count.frames = 0;
    count.pulseSkip = 0;
    }
}

void chkStateUpdates(){
  
  EVERY_N_MILLIS(500){
    static uint8_t statusLast = SIG_NONE;
    static uint8_t serialStatusLast = SENT_SER_NONE;
    static bool supplyVoltageLast = false;
    static bool overcurrentLast = false;
    const uint8_t * img_imp = img_0_impulse;
    const uint8_t * img_sig = img_0_impulse;
    

    if(sig.status != statusLast){
      }
    if(sig.supplyVoltage != supplyVoltageLast){infoMsg(sig.supplyVoltage ? "Versorgungsspannung: an" : "Versorgungsspannung: aus");}
    if(sig.overcurrent != overcurrentLast){errorMsg(sig.overcurrent ? "Kurzschluss!" : "Fehler aufgehoben");}

    statusLast = sig.status;
    serialStatusLast = sig.serialStatus;
    supplyVoltageLast = sig.supplyVoltage;
    overcurrentLast = sig.overcurrent;
  }
}

void shortGuard(){
  static bool faultLast = HIGH;  
  EVERY_N_MILLIS(100){
    bool fault = digitalRead(FAULT_PIN);
    if(fault == LOW && faultLast == HIGH){
      errorMsg("Kurzschluss 5V zu Gnd!");
      resetSensor(); 
      sig.ovc_protect_ms = millis()+15000; 
      sig.supplyVoltage = false; 
      sig.overcurrent = true;
    }
    else if(fault == HIGH && faultLast == LOW){
      sig.overcurrent = false;
    }

    faultLast = fault;
  }
}
