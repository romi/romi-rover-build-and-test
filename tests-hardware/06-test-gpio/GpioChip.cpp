#include <stdexcept>
#include <r.h>
#include "GpioChip.h"

namespace romi {

        GpioChip::GpioChip(uint num) : chip_(nullptr)
        {
                chip_ = gpiod_chip_open_by_number(num);
                if (chip_ == nullptr) {
                        char buffer[128];
                        strerror_r(errno, buffer, sizeof(buffer));
                        r_err("Failed to open the chip: %s", buffer);
                        throw std::runtime_error("Failed to open the GPIO chip");
                }
                r_info("Opened GPIO chip '%s'", gpiod_chip_name(chip_));
        }

        GpioChip::~GpioChip()
        {
                if (chip_)
                        gpiod_chip_close(chip_);
        }
}
