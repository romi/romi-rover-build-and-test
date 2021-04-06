#ifndef _LIBROMI_GPIO_INPUT_H_
#define _LIBROMI_GPIO_INPUT_H_

#include "GpioLine.h"

namespace romi {

        class GpioInput : public GpioLine
        {
        public:
                GpioInput(GpioChip& chip, uint offset);
                virtual ~GpioInput() override = default;
                
                bool get();
        };
}

#endif // _LIBROMI_GPIO_INPUT_H_
