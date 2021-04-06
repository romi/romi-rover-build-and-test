#include <stdexcept>
#include <r.h>
#include "GpioLine.h"

namespace romi {

        GpioLine::GpioLine(GpioChip& chip, uint offset) : line_(nullptr)
        {
                line_ = gpiod_chip_get_line(chip.ptr(), offset);
                if (line_ == nullptr) {
                        throw_error("Failed to open the GPIO line");
                }
        }

        GpioLine::~GpioLine()
        {
                if (line_)
                        gpiod_line_release(line_);
        }

        void GpioLine::throw_error(const char *what)
        {
                char errno_message[128];
                strerror_r(errno, errno_message, sizeof(errno_message));
                r_err("%s: %s", what, errno_message);
                throw std::runtime_error(what);
        }
}

