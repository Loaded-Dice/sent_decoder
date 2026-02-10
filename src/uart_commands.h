#ifndef UART_COMMANDS_H
#define UART_COMMANDS_H

#include "config.h"
#include "sent_types.h"
#include <ArduinoJson.h>

// Command parsing and execution
void parseAndExecuteCommand(String cmd);
String validateCommand(String command, String param, UartCommandState& state, bool& vccOn, bool& vccOff,
                       bool& startCmd, bool& stopCmd, bool& getinfoCmd, bool& resetCmd, bool& restartCmd,
                       bool& ovcCmd, bool& helpCmd, bool& identifyCmd);

// Command handlers
void cmd_vcc(String param);
void cmd_stop();
void cmd_start();
void cmd_getinfo();
void cmd_reset();
void cmd_restart();
void cmd_ch1(String param);
void cmd_ch2(String param);
void cmd_supp(String param);
void cmd_delay(String param);
void cmd_help();
void cmd_ovc(String param);
void cmd_identify();

// Output functions
void sendContinuousOutput();
void sendCompleteInfo();
void addChannelToJson(JsonObject obj, const Channel& ch, ChannelOutputMode mode);

#endif // UART_COMMANDS_H
