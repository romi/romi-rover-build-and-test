
#ifndef _ZAZAI_I_ENCODER_H
#define _ZAZAI_I_ENCODER_H

#include "fixed.h"

class IEncoder
{
public:
        virtual ~IEncoder() = default;
        virtual int32_t get_position() = 0;
        virtual double get_speed(double dt) = 0;
};

#endif // _ZAZAI_I_ENCODER_H
