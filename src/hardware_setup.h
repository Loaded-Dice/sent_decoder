#ifndef HARDWARE_SETUP_H
#define HARDWARE_SETUP_H

#include "config.h"
#include "sent_types.h"

void sent_isr_handler(void* arg);
void core1_loop(void* pvParameters);
void register_isr_on_core0(void* arg);
void setupGptimer0();
void init_pcnt(void);
void setupTft();

#endif // HARDWARE_SETUP_H
