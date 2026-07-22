#include "json_output.h"
#include "globals.h"
#include "misc_functions.h"
#include "info_strings.h"

void createFrameDetectJson() {
  StaticJsonDocument<512> doc;
  doc["title"]        = "signal";
  doc["sync_us"]        = (float)sig.syncLen / (float)ESP_TICKS_PER_US;
  doc["f_pulses"]       = sig.numPulses;
  doc["frame_len_us"]   = (float)sig.frameLen / (float)ESP_TICKS_PER_US;
  doc["tick_len_us"]    = (float)sig.tickLen / (float)ESP_TICKS_PER_US;
  doc["tick_nominal"]   = sig.tickNominal_us;
  doc["tickPerTick"]    = sig.tickNominal_ticks;
  doc["syncNominal"]    = sig.syncNominal_us;
  doc["constLen"]       = sig.frameLenConst;
  doc["dataNibbles"]    = sig.dataNibbles;

  Serial.print('#'); 
  serializeJson(doc, Serial);
  Serial.print('~'); 
  Serial.println();
}

void printSerialSentInfoJson() {
  StaticJsonDocument<1024> doc;
  doc["title"]        = "serial";
  doc["Num Serial IDs"] = unique;
  JsonArray ID_list = doc.createNestedArray("ID_list");
  
  for (uint8_t i = 0; i < unique; i++) {
    JsonObject entry = ID_list.createNestedObject();
    entry["ID"] = getCodeAsHex(uniqueIds[i].messageId);
    entry["ID desc"] = getMessageIdDefinition(uniqueIds[i].messageId);
    entry["code"]    = getCodeAsHex(uniqueIds[i].code);
  }

  if (sentMeta.manufacturer != -1)  doc["manufacturer"] = getManufacturerDefinition(sentMeta.manufacturer);
  if (sentMeta.status != -1)   doc["status"] = getErrorDescription(sentMeta.status);
  if (sentMeta.sentRev != -1)    doc["sentRevision"] = getSentRev(sentMeta.sentRev);
  if (strlen(sentMeta.ascii) > 1)    doc["ascii"] = sentMeta.ascii;
  if (sentMeta.configCode != -1)    doc["configCode"] = sentMeta.configCode;

  Serial.print('#'); 
  serializeJson(doc, Serial);
  Serial.print('~'); 
  Serial.println(); 
}

void printAllChInfoAsJson() {
  StaticJsonDocument<512> doc;
  doc["title"] = "channel";

  JsonArray channels = doc.createNestedArray("channels");

  JsonObject ch1 = channels.createNestedObject();
  fillChInfoJson(ch1, chFrame.ch1, "Fast_Ch1");

  JsonObject ch2 = channels.createNestedObject();
  fillChInfoJson(ch2, chFrame.ch2, "Fast_Ch2");

  JsonObject supp = channels.createNestedObject();
  fillChInfoJson(supp, chFrame.ch_supp[0][0], "Supplementary_Ch");
  Serial.print('#'); 
  serializeJson(doc, Serial);
  Serial.print('~'); 
  Serial.print(deltaRoll); 
  
  Serial.println(); 
}

void fillChInfoJson(JsonObject obj, const Channel& ch, const char* name) {
  obj["name"] = name;
  obj["type"] = chTypeDesc[ch.type];

  if (ch.type == ch_undefined) return;

  obj["bit depth"] = ch.depth;

  obj["Transfer Function Status"] =
    (ch.valTrans == VALTRANS_OK)   ? "successful" :
    (ch.valTrans == VALTRANS_NONE) ? "none" :
                                      "Error (init)";

  if (ch.nodes.x1done || ch.nodes.x2done || ch.nodes.y1done || ch.nodes.y2done) {
    JsonObject nodes = obj.createNestedObject("nodes(raw)");
    if (ch.nodes.x1done) nodes["x1"] = ch.nodes.x1;
    if (ch.nodes.x2done) nodes["x2"] = ch.nodes.x2;
    if (ch.nodes.y1done) nodes["y1"] = ch.nodes.y1;
    if (ch.nodes.y2done) nodes["y2"] = ch.nodes.y2;
  }

  if (ch.nodes_f.x1done || ch.nodes_f.x2done || ch.nodes_f.y1done || ch.nodes_f.y2done) {
    JsonObject nodes_f = obj.createNestedObject("nodes(float)");
    if (ch.nodes_f.x1done) nodes_f["x1"] = ch.nodes_f.x1;
    if (ch.nodes_f.x2done) nodes_f["x2"] = ch.nodes_f.x2;
    if (ch.nodes_f.y1done) nodes_f["y1"] = ch.nodes_f.y1;
    if (ch.nodes_f.y2done) nodes_f["y2"] = ch.nodes_f.y2;
  }
}

void jsonSentValues(){
      StaticJsonDocument<512> doc;
      doc["title"]        = "val";
      if(chFrame.ch1.type != ch_undefined ){
        JsonObject ch1 = doc.createNestedObject("Ch1");
        chValJson(ch1, chFrame.ch1);
      }
      if(chFrame.ch2.type != ch_undefined ){
        JsonObject ch2 = doc.createNestedObject("Ch2");
        chValJson(ch2, chFrame.ch2);
      }
      if(chFrame.ch_supp[0][0].type != ch_undefined ){
        JsonObject supp = doc.createNestedObject("Supplementary");
        chValJson(supp, chFrame.ch_supp[0][0]);
      }
      
      doc["FPS"] = count.frames_perSec;
      doc["CRC fail [%]"] = count.crcFail_percent;
      Serial.print('#'); 
      serializeJson(doc, Serial);
      Serial.print('~'); 
      Serial.println(); 
}

void chValJson(JsonObject obj, const Channel& ch) {
  if(ch.type == ch_zero || ch.type == ch_position_sensor_specific || ch.type == ch_position_multi_dim || ch.valTrans == VALTRANS_NONE){
    obj["RAW"] = getValAs0xHex(ch.raw,ch.depth);

  }
  else{
    obj["Val"] = ch.value;
    obj["unit"] = ch.unit;
    obj["RAW"] = getValAs0xHex(ch.raw,ch.depth);
  }
}

void errorMsgJson(String msg){msgJson("error",msg);}

void infoMsgJson(String msg){msgJson("info",msg);}

void msgJson(String type, String msg){
  StaticJsonDocument<128> doc;
  doc["title"]        = type;
  doc["msg"]          = msg;
  Serial.print('#'); 
  serializeJson(doc, Serial);
  Serial.print('~'); 
  Serial.println(); 
}
