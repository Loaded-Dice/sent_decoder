#ifndef GLOBALS_H
#define GLOBALS_H

#include "sent_types.h"

// When true, a temperature channel with a fixed standard/high transfer function
// that nonetheless reports X1/X2 and Y1/Y2 nodes is switched to the special
// (linear) transfer using those nodes, instead of the fixed function.
const bool special_transf_when_XY_provided = true;

// Utility function declared here so misc_functions.cpp can use it
void clearRingBuff();

#endif // GLOBALS_H
