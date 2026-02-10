#ifndef INPUTS_H
#define INPUTS_H

#include "config.h"
#include "sent_types.h"

// Encoder functions
int16_t get_encoder_value(void);
bool processEncBtn();
void handleInputs();
int getEncDelta();
bool getEncRot();
bool getEncBtn();

#endif // INPUTS_H
