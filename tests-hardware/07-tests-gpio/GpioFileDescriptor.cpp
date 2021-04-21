#include <stdexcept>
#include <r.h>
#include "GpioFileDescriptor.h"

namespace romi {

        GpioFileDescriptor::~GpioFileDescriptor()
        {
                if (fd_ >= 0)
                        close(fd_);
        }
        
        void GpioFileDescriptor::throw_error(const char *what)
        {
                char errno_message[128];
                strerror_r(errno, errno_message, sizeof(errno_message));
                r_err("%s: %s", what, errno_message);
                throw std::runtime_error(what);
        }
}

