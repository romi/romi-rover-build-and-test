#ifndef _LIBROMI_GPIO_INPUT_H_
#define _LIBROMI_GPIO_INPUT_H_

#include <stdint.h>
#include "GpioFileDescriptor.h"
#include "GpioChip.h"

namespace romi {

        class GpioInput : public GpioFileDescriptor
        {
        public:
                GpioInput(GpioChip& chip, uint32_t offset);
                ~GpioInput() override = default;
                
                bool get(bool& value);
        };
}

#endif // _LIBROMI_GPIO_INPUT_H_
