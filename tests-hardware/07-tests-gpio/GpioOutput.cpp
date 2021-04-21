#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include "GpioOutput.h"

namespace romi {

        GpioOutput::GpioOutput(GpioChip& chip, uint32_t offset)
                : GpioFileDescriptor()
        {
                struct gpiohandle_request request;
                memset(&request, 0, sizeof(struct gpiohandle_request));
                request.lineoffsets[0] = offset;
                request.flags = GPIOHANDLE_REQUEST_OUTPUT;
                request.lines = 1;

                if (ioctl(chip.get_fd(), GPIO_GET_LINEHANDLE_IOCTL, &request) < 0) {
                        throw_error("GpioOutput: Get line handle failed");
                }

                fd_ = request.fd;
        }
        
        bool GpioOutput::set(bool on)
        {
                bool success = true;
                struct gpiohandle_data data;
                data.values[0] = on? 1 : 0;
		if (ioctl(fd_, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0)
                        success = false;
                return success;
	}
}

