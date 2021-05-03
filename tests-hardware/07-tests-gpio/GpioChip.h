#ifndef _LIBROMI_GPIO_CHIP_H_
#define _LIBROMI_GPIO_CHIP_H_

#include "GpioFileDescriptor.h"

namespace romi {

        class GpioChip : public GpioFileDescriptor
        {
        public:
                GpioChip();
                ~GpioChip() override = default;
        };
}

#endif // _LIBROMI_GPIO_CHIP_H_
