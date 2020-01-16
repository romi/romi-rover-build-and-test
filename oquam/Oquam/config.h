#ifndef _OQUAM_CONFIG_H_
#define _OQUAM_CONFIG_H_

/*
                  Board        Encoders   Limits   
gShield           Uno          0          1
Ext. controller   Mega 2560    1          1
RAMPS             Mega 2560   

*/

#define USE_GSHIELD 0
#define USE_EXT_CONTROLLER 1

#if USE_GSHIELD
#include "gshield.h"
#endif

#if USE_EXT_CONTROLLER
#include "extctrlr.h"
#endif


//#define PRESCALING 8
//#define FREQUENCY_STEPPER 10000
//#define INTERRUPTS_PER_MILLISECOND 10

#endif // _OQUAM_CONFIG_H_
