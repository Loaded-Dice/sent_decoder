#ifndef OUTPUT_H
#define OUTPUT_H

#include "config.h"
#include "sent_types.h"

void infoMsg(String msg);
void errorMsg(String msg);

// Output control functions
void outputSigCtrl();
void outputSerialIDs();
void outputValues();

// Message functions
void errorMsg(String msg);
void infoMsg(String msg);

// Serial output (text)
void printSerialSentInfos();

#endif // OUTPUT_H
