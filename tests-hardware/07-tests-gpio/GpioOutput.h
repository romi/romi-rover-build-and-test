#ifndef _LIBROMI_GPIO_OUTPUT_H_
#define _LIBROMI_GPIO_OUTPUT_H_

#include <stdint.h>
#include "GpioFileDescriptor.h"
#include "GpioChip.h"

namespace romi {

        class GpioOutput : public GpioFileDescriptor
        {
        public:
                GpioOutput(GpioChip& chip, uint32_t offset);
                ~GpioOutput() override = default;
                
                bool set(bool value);
        };
}

#endif // _LIBROMI_GPIO_OUTPUT_H_
