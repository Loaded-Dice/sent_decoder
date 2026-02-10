#ifndef VALUE_TRANSFER_H
#define VALUE_TRANSFER_H

#include "config.h"
#include "sent_types.h"

// Transfer configuration
void chooseTransferConfig(Channel* ch, String chName);
void setChUnit(Channel* ch);

// Linear transfer configuration
void configXLinTransfer(Channel* ch, transferCoeff tf);
void configYLinTransfer(Channel* ch, bool useDefaultY);

// Calculation functions
float calcXlin(transferCoeff tfc, uint16_t X_d);
int16_t signedTwosComplement(uint16_t input, uint8_t bitDepth);

// Value transfer functions
void valTransfer(Channel* ch);
uint32_t checkValueError(Channel* ch);
void linearTransfer(Channel* ch);
void linearTemperatureTransfer(Channel* ch);

#endif // VALUE_TRANSFER_H
