#ifndef _LIBROMI_GPIO_FD_H_
#define _LIBROMI_GPIO_FD_H_

namespace romi {

        class GpioFileDescriptor
        {
        protected:
                int fd_;
                
                void throw_error(const char *what);
                
        public:
                GpioFileDescriptor() : fd_(-1) {}
                virtual ~GpioFileDescriptor();

                int get_fd() {
                        return fd_;
                }
        };
}

#endif // _LIBROMI_GPIO_FD_H_
