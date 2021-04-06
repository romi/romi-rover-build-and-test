#ifndef _LIBROMI_GPIO_OUTPUT_H_
#define _LIBROMI_GPIO_OUTPUT_H_

#include "GpioLine.h"

namespace romi {

        class GpioOutput : public GpioLine
        {
        public:
                GpioOutput(GpioChip& chip, uint offset);
                virtual ~GpioOutput() override = default;
                
                void set(bool value);
        };
}

#endif // _LIBROMI_GPIO_OUTPUT_H_
