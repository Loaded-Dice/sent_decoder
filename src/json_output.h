#ifndef JSON_OUTPUT_H
#define JSON_OUTPUT_H

#include "config.h"
#include "sent_types.h"

// JSON output functions
void createFrameDetectJson();
void printSerialSentInfoJson();
void printAllChInfoAsJson();
void fillChInfoJson(JsonObject obj, const Channel& ch, const char* name);
void jsonSentValues();
void chValJson(JsonObject obj, const Channel& ch);

// Message functions
void errorMsgJson(String msg);
void infoMsgJson(String msg);
void msgJson(String type, String msg);

#endif // JSON_OUTPUT_H
