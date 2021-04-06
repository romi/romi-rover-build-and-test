#include <stdexcept>
#include <r.h>
#include "GpioInput.h"

namespace romi {

        GpioInput::GpioInput(GpioChip& chip, uint offset)
                : GpioLine(chip, offset)
        {
                if (gpiod_line_request_input(line_, kConsumer)) {
                        throw_error("Failed to set the GPIO line to input");
                }
        }

        bool GpioInput::get()
        {
                int value = gpiod_line_get_value(line_);
                if (value == -1) {
                        throw_error("Failed to read the GPIO value");
                }
                return value == 0? false : true;
        }
}

