
#ifndef _MOTORCONTROLLER_I_FILTER_H
#define _MOTORCONTROLLER_I_FILTER_H

#include <stdint.h>

class IFilter
{
public:

        virtual ~IFilter() = default;
        virtual int16_t compute(int16_t x) = 0;
};

#endif // _MOTORCONTROLLER_I_FILTER_H
