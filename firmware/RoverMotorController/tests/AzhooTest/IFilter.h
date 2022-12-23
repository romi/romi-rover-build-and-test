
#ifndef _ZAZAI_I_FILTER_H
#define _ZAZAI_I_FILTER_H

#include "ITimerListener.h"

class IFilter
{
public:

        virtual ~IFilter() = default;
        virtual double compute(double x) = 0;
};

#endif // _ZAZAI_I_FILTER_H
