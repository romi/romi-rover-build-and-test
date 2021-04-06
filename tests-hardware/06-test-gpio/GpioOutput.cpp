#include <stdexcept>
#include <r.h>
#include "GpioOutput.h"

namespace romi {

        GpioOutput::GpioOutput(GpioChip& chip, uint offset)
                : GpioLine(chip, offset)
        {
                if (gpiod_line_request_output(line_, kConsumer, 0) != 0) {
                        throw_error("Failed to set the GPIO line to output");
                }
        }

        void GpioOutput::set(bool on)
        {
                int value = on? 1 : 0;
                if (gpiod_line_set_value(line_, value) != 0) {
                        throw_error("Failed to set the GPIO value");
                }
        }
}

