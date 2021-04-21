#include <stdexcept>
#include <unistd.h>
#include <r.h>
#include "GpioChip.h"

namespace romi {

        GpioChip::GpioChip() : GpioFileDescriptor()
        {
                fd_ = open("/dev/gpiochip0", O_RDONLY);
                if (fd_ == -1) {
                        throw_error("Failed to open the GPIO chip");
                }
                r_info("Opened GPIO chip");
        }
}
