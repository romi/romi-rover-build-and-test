#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include "GpioInput.h"

namespace romi {

        GpioInput::GpioInput(GpioChip& chip, uint32_t offset)
                : GpioFileDescriptor()
        {
                struct gpiohandle_request request;
                memset(&request, 0, sizeof(struct gpiohandle_request));
                request.lineoffsets[0] = offset;
                request.flags = GPIOHANDLE_REQUEST_INPUT;
                request.lines = 1;

                if (ioctl(chip.get_fd(), GPIO_GET_LINEHANDLE_IOCTL, &request) < 0) {
                        throw_error("GpioInput: Get line handle failed");
                }

                fd_ = request.fd;
        }

        bool GpioInput::get(bool& value)
        {
                bool success = true;
                struct gpiohandle_data data;
		if (ioctl(fd_, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0)
                        success = false;
                else 
                        value = (data.values[0] == 0)? false : true;
                return success;
        }
}

