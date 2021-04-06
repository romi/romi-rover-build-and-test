#ifndef _LIBROMI_GPIO_LINE_H_
#define _LIBROMI_GPIO_LINE_H_

#include <gpiod.h>
#include "GpioChip.h"

namespace romi {

        class GpioLine
        {
        protected:
                static constexpr const char *kConsumer = "Romi";
                struct gpiod_line *line_;

                void throw_error(const char *what);
                
        public:
                GpioLine(GpioChip& chip, uint offset);
                virtual ~GpioLine();
        
                struct gpiod_line *ptr() {
                        return line_;
                }
                
        private:
                GpioLine(const GpioLine& other) = default;
                GpioLine& operator=(const GpioLine& other) = default;
        };
}

#endif // _LIBROMI_GPIO_LINE_H_
