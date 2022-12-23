
#ifndef _ZAZAI_I_ARDUINO_H
#define _ZAZAI_I_ARDUINO_H

#include "ITimer.h"

class IArduino
{
public:
        virtual ~IArduino() = default;
        virtual ITimer& timer1() = 0;
        virtual bool pin_is_high(int id) = 0;
};

#endif // _ZAZAI_I_ARDUINO_H
