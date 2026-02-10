#ifndef INFO_STRINGS_H
#define INFO_STRINGS_H

#include "config.h"
#include "sent_types.h"

// Message ID definitions
String getMessageIdDefinition(uint8_t messageId);

// Manufacturer definitions
String getManufacturerDefinition(uint16_t manufacturerCode);

// Code definitions
String getCodeDefinition(uint8_t messageId, uint16_t code);
String getSentRev(uint16_t code);
String getErrorDescription(uint16_t code);

// Channel information
void printAllChInfo();
String getChInfo(Channel ch);

#endif // INFO_STRINGS_H
