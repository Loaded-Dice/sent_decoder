#include "uart_commands.h"
#include "globals.h"
#include "misc_functions.h"
#include "json_output.h"
#include "output.h"

// Parse, validate and execute command string
void parseAndExecuteCommand(String cmdLine) {
  cmdLine.trim();
  if (cmdLine.length() == 0) return;
  
  // Create temporary state for validation
  UartCommandState newState = uartCmd;
  bool vccOn = false;
  bool vccOff = false;
  bool startCmd = false;
  bool stopCmd = false;
  bool getinfoCmd = false;
  bool resetCmd = false;
  bool restartCmd = false;
  bool ovcCmd = false;
  bool helpCmd = false;
  bool identifyCmd = false;
  String errorMsg = "";
  
  // Parse all commands first
  int startIdx = 0;
  String currentCmd = "";
  String currentParam = "";
  bool expectingParam = false;
  
  for (int i = 0; i <= cmdLine.length(); i++) {
    char c = (i < cmdLine.length()) ? cmdLine[i] : ' ';
    
    if (c == ' ' || i == cmdLine.length()) {
      if (startIdx < i) {
        String token = cmdLine.substring(startIdx, i);
        
        if (token.startsWith("-")) {
          // Validate previous command
          if (currentCmd.length() > 0) {
            String err = validateCommand(currentCmd, currentParam, newState, vccOn, vccOff, startCmd, stopCmd, 
                                        getinfoCmd, resetCmd, restartCmd, ovcCmd, helpCmd, identifyCmd);
            if (err.length() > 0) { errorMsg = err; break; }
          }
          currentCmd = token.substring(1);
          currentParam = "";
          expectingParam = true;
        } 
        else if (expectingParam) { currentParam = token; expectingParam = false; }
      }
      startIdx = i + 1;
    }
  }
  
  // Validate last command
  if (errorMsg.length() == 0 && currentCmd.length() > 0) {
    errorMsg = validateCommand(currentCmd, currentParam, newState, vccOn, vccOff, startCmd, stopCmd,
                               getinfoCmd, resetCmd, restartCmd, ovcCmd, helpCmd, identifyCmd);
  }
  
  // Check for validation errors
  if (errorMsg.length() > 0) { errorMsgJson(errorMsg); return; }
  
  // Additional validation: -start requires at least one channel
  if (startCmd && newState.ch1Mode == CH_OFF && newState.ch2Mode == CH_OFF && newState.suppMode == CH_OFF) {
    errorMsgJson("Keine Channels aktiviert - verwende -ch1/-ch2/-supp vor -start");
    return;
  }
  
  // All valid - execute commands in priority order
  if (vccOn) { startSensor(); if (!sig.supplyVoltage) { errorMsgJson("VCC Aktivierung fehlgeschlagen (OVC Protection?)"); return; } }
  if (vccOff) { resetSensor(); }
  if (restartCmd) { resetSensor(); delay(100); startSensor(); if (!sig.supplyVoltage) { errorMsgJson("Neustart fehlgeschlagen (OVC Protection?)"); return; } }
  if (resetCmd) { detectSignal(); }
  if (ovcCmd) { sig.overcurrent = false; sig.ovc_protect_ms = 0; }
  
  // Apply new state
  uartCmd.ch1Mode = newState.ch1Mode;
  uartCmd.ch2Mode = newState.ch2Mode;
  uartCmd.suppMode = newState.suppMode;
  uartCmd.outputDelay_ms = newState.outputDelay_ms;
  
  if (stopCmd) { uartCmd.outputActive = false; uartCmd.startRequested = false; }
  if (startCmd) { uartCmd.startRequested = true; }
  if (getinfoCmd) { 
    if (!sig.supplyVoltage) { errorMsgJson("Kein Signal erkannt - Bitte zuerst -vcc on"); return; }
    if (sig.serialStatus == SENT_SER_LOOP && sig.status == SIG_OK) { sendCompleteInfo(); } 
    else { uartCmd.sendFrameInfo = true; }
  }
  
  if (helpCmd) { cmd_help(); return; }
  if (identifyCmd) { infoMsgJson(IDENTIFY); return; }
  
  infoMsgJson("OK");
}

// Validate single command and update state
String validateCommand(String command, String param, UartCommandState& state, bool& vccOn, bool& vccOff,
                       bool& startCmd, bool& stopCmd, bool& getinfoCmd, bool& resetCmd, bool& restartCmd,
                       bool& ovcCmd, bool& helpCmd, bool& identifyCmd) {
  command.toLowerCase();
  param.toLowerCase();
  
  if (command == "vcc") {
    if (param == "on") { vccOn = true; tftState = TFTSTATE_DASHBOARD ; } 
    else if (param == "off") { vccOff = true; tftState = TFTSTATE_MENU;} 
    else { return "Invalid parameter for -vcc (use: on/off)"; }
  } 
  else if (command == "stop") { stopCmd = true; } 
  else if (command == "start") { startCmd = true; } 
  else if (command == "getinfo") { getinfoCmd = true; } 
  else if (command == "reset") { resetCmd = true; } 
  else if (command == "restart") { restartCmd = true; } 
  else if (command == "ch1") {
    if (param == "val") { state.ch1Mode = CH_VAL; } 
    else if (param == "raw") { state.ch1Mode = CH_RAW; } 
    else if (param == "off") { state.ch1Mode = CH_OFF; } 
    else { return "Invalid parameter for -ch1 (use: val/raw/off)"; }
  } 
  else if (command == "ch2") {
    if (param == "val") { state.ch2Mode = CH_VAL; } 
    else if (param == "raw") { state.ch2Mode = CH_RAW; } 
    else if (param == "off") { state.ch2Mode = CH_OFF; } 
    else { return "Invalid parameter for -ch2 (use: val/raw/off)"; }
  } 
  else if (command == "supp" || command == "supplementary") {
    if (param == "val") { state.suppMode = CH_VAL; } 
    else if (param == "raw") { state.suppMode = CH_RAW; } 
    else if (param == "off") { state.suppMode = CH_OFF; } 
    else { return "Invalid parameter for -supp (use: val/raw/off)"; }
  } 
  else if (command == "delay") {
    if (param.length() == 0) { state.outputDelay_ms = 200; } 
    else {
      int delayVal = param.toInt();
      if (delayVal < 50) { state.outputDelay_ms = 50; } 
      else if (delayVal > 10000) { state.outputDelay_ms = 10000; } 
      else { state.outputDelay_ms = delayVal; }
    }
  } 
  else if (command == "ovc") {
    if (param == "reset") { ovcCmd = true; } 
    else { return "Invalid parameter for -ovc (use: reset)"; }
  } 
  else if (command == "help") { helpCmd = true; } 
  else if (command == "identify") { identifyCmd = true; } 
  else { return "Unknown command: -" + command; }
  
  return "";
}

// Command: -help
void cmd_help() {
  String helpText = "Verfügbare Commands:\n"
    "-vcc on/off       Versorgungsspannung an/aus\n"
    "-start            Kontinuierliche Ausgabe starten\n"
    "-stop             Kontinuierliche Ausgabe stoppen\n"
    "-getinfo          Einmalige Metadaten-Ausgabe\n"
    "-reset            Signal neu analysieren\n"
    "-restart          VCC off-on-reset\n"
    "-ch1 val/raw/off  Channel 1 Ausgabe-Modus\n"
    "-ch2 val/raw/off  Channel 2 Ausgabe-Modus\n"
    "-supp val/raw/off Supplementary Channel Modus\n"
    "-delay <ms>       Ausgabe-Intervall (50-10000ms, default:200)\n"
    "-ovc reset        Overcurrent Protection zurücksetzen\n"
    "-identify         Decoder-Identifikation anzeigen\n"
    "-help             Diese Hilfe anzeigen\n"
    "\n"
    "Beispiel: -vcc on -ch1 val -ch2 raw -delay 100 -start";
  
  infoMsgJson(helpText);
}

// Send complete info (signal + serial + channels)
void sendCompleteInfo() {
  createFrameDetectJson();
  delay(10);
  
  if (sig.serialStatus == SENT_SER_LOOP) {
    printSerialSentInfoJson();
    delay(10);
  }
  
  printAllChInfoAsJson();
}

// Send continuous output based on current settings
void sendContinuousOutput() {
  if (!uartCmd.outputActive) return;
  if (sig.status != SIG_OK) return;
  
  uint32_t now = millis();
  if (now - uartCmd.lastOutput_ms < uartCmd.outputDelay_ms) return;
  
  uartCmd.lastOutput_ms = now;
  
  StaticJsonDocument<512> doc;
  doc["title"] = "val";
  bool anyChannelAdded = false;
  
  // Channel 1
  if (uartCmd.ch1Mode != CH_OFF && chFrame.ch1.type != ch_undefined) {
    JsonObject ch1 = doc.createNestedObject("Ch1");
    addChannelToJson(ch1, chFrame.ch1, uartCmd.ch1Mode);
    anyChannelAdded = true;
  }
  
  // Channel 2
  if (uartCmd.ch2Mode != CH_OFF && chFrame.ch2.type != ch_undefined) {
    JsonObject ch2 = doc.createNestedObject("Ch2");
    addChannelToJson(ch2, chFrame.ch2, uartCmd.ch2Mode);
    anyChannelAdded = true;
  }
  
  // Supplementary Channel
  if (uartCmd.suppMode != CH_OFF && chFrame.ch_supp[0][0].type != ch_undefined) {
    JsonObject supp = doc.createNestedObject("Supp");
    addChannelToJson(supp, chFrame.ch_supp[0][0], uartCmd.suppMode);
    anyChannelAdded = true;
  }
  
  if (anyChannelAdded) {
    doc["FPS"] = count.frames_perSec;
    doc["CRC_fail_%"] = count.crcFail_percent;
    
    Serial.print('#');
    serializeJson(doc, Serial);
    Serial.print('~');
    Serial.println();
  }
}

// Helper: Add channel data to JSON based on mode
void addChannelToJson(JsonObject obj, const Channel& ch, ChannelOutputMode mode) {
  // Check for out-of-range error (12-bit: 4089-4095, 14-bit: 16353-16383, 16-bit: 65521-65535)
  bool outOfRange = false;
  if (ch.depth == 12 && ch.raw >= 4089) { outOfRange = true; } 
  else if (ch.depth == 14 && ch.raw >= 16353) { outOfRange = true; } 
  else if (ch.depth == 16 && ch.raw >= 65521) { outOfRange = true; }
  
  if (outOfRange) {
    // Always output RAW on error + send error message
    obj["RAW"] = getCodeAsHex(ch.raw);
    obj["unit"] = ch.unit;
    obj["error"] = "Out of range";
    errorMsgJson("Channel out of range: RAW=" + getCodeAsHex(ch.raw));
    return;
  }
  
  // Normal output based on mode
  if (mode == CH_RAW) { obj["RAW"] = getCodeAsHex(ch.raw); obj["unit"] = ch.unit; } 
  else if (mode == CH_VAL) {
    // Try to output value, fallback to RAW if transfer function not available
    if (ch.valTrans == VALTRANS_OK) { obj["Val"] = ch.value; obj["unit"] = ch.unit; obj["RAW"] = getCodeAsHex(ch.raw); } 
    else { obj["RAW"] = getCodeAsHex(ch.raw); obj["unit"] = ch.unit; }
  }
}
