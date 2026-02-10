#ifndef CHANNEL_CONFIG_H
#define CHANNEL_CONFIG_H

#include "config.h"
#include "sent_types.h"

// Channel configuration based on 12-bit code
void setChConfig(ChannelConfig* cfg, uint16_t code12bit);

#endif // CHANNEL_CONFIG_H
