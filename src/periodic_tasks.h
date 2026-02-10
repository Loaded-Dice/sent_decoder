#ifndef PERIODIC_TASKS_H
#define PERIODIC_TASKS_H

#include "config.h"
#include "sent_types.h"

// Serial communication
void serialRead();
void serialHandler(String msg);
void periodicUART();
String getSignalStatusMsg();

// Status monitoring
void calcStats();
void chkStateUpdates();
void shortGuard();

#endif // PERIODIC_TASKS_H
