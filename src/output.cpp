#include "output.h"
#include "globals.h"
#include "json_output.h"
#include "info_strings.h"

void outputSigCtrl(){
  if(JSON_OUTPUT){createFrameDetectJson();}
}

void outputSerialIDs(){
  if(JSON_OUTPUT){printSerialSentInfoJson();Serial.println(); printAllChInfoAsJson();}
  else{printSerialSentInfos();Serial.println(); printAllChInfo();}
}

uint32_t serial_delay = 155;

void outputValues(){
  static uint32_t printTimer = 0;
  if (sig.status == SIG_OK && sig.serialStatus == SENT_SER_LOOP && millis() >printTimer + serial_delay){
    printTimer = millis();
    if(JSON_OUTPUT){ jsonSentValues(); }
  }
}

void errorMsg(String msg){if(JSON_OUTPUT){ errorMsgJson(msg); }}

void infoMsg(String msg){ if(JSON_OUTPUT){ infoMsgJson(msg);}}

void printSerialSentInfos(){
  
  Serial.println();
  Serial.println("---------------=================={INFOS}==================---------------");
  Serial.println();
    Serial.println("Assigned:");

    for (uint8_t i = 0; i < unique; i++){
      if(uniqueIds[i].assigned){ 
        Serial.print( getMessageIdDefinition(uniqueIds[i].messageId) ); 
        Serial.print('\t'); 
        Serial.print('\t'); 
        Serial.print(getCodeDefinition(uniqueIds[i].messageId, uniqueIds[i].code)); 
        Serial.println();
      }}
    Serial.println("-------------------------------------");
    Serial.println("Not assigned:");
    for (uint8_t i = 0; i < unique; i++){
      if(!uniqueIds[i].assigned){ 
      Serial.print( getMessageIdDefinition(uniqueIds[i].messageId) ); 
      Serial.print('\t'); 
      
      Serial.print(getCodeDefinition(uniqueIds[i].messageId, uniqueIds[i].code)); 
      Serial.print(uniqueIds[i].fixedVal ? "\tFester Wert": "\tVariablel");
      Serial.println();
      }}
    Serial.println("-------------------------------------");
    Serial.println();

    if(sentMeta.manufacturer != -1){Serial.print("Hersteller: ");   Serial.print('\t'); Serial.println(getManufacturerDefinition(sentMeta.manufacturer));}
    if(sentMeta.status != -1){      Serial.print("Status/Fehler: ");Serial.print('\t'); Serial.println(getErrorDescription(sentMeta.status));}
    if(sentMeta.sentRev != -1){     Serial.print("Sent Revision: ");Serial.print('\t'); Serial.println(getSentRev(sentMeta.sentRev));}
    if(strlen(sentMeta.ascii) > 1){Serial.print("Ascii: ");Serial.print('\t'); Serial.println(sentMeta.ascii);}
    if(sentMeta.configCode != -1){     Serial.print("Ch. Config code: ");Serial.print('\t'); Serial.println(sentMeta.configCode);}
    Serial.println();
}
