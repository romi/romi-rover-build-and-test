#ifndef _LIBROMI_GPIO_CHIP_H_
#define _LIBROMI_GPIO_CHIP_H_

#include <gpiod.h>

namespace romi {

        class GpioChip
        {
        protected:
                struct gpiod_chip *chip_;

                
        public:
                GpioChip(uint num);
                ~GpioChip();
        
                struct gpiod_chip *ptr() {
                        return chip_;
                }

        private:
                GpioChip(const GpioChip& other) = default;
                GpioChip& operator=(const GpioChip& other) = default;

        };
}

#endif // _LIBROMI_GPIO_CHIP_H_
