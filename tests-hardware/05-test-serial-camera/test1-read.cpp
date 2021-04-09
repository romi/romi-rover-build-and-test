#include <stdlib.h>
#include <r.h>

int main()
{
        serial_t *serial = new_serial("/dev/serial0", 115200, 0);
        if (serial == nullptr) {
                r_err("Failed to open serial device");
                exit(1);
        }

        membuf_t *buffer = new_membuf();
        while (true) {
                const char *request = serial_readline(serial, buffer);
                if (request != nullptr) {
                        r_info("%s", request);
                } else {
                        r_warn("serial_readline returned null");
                        break;
                }
                serial_print(serial, "#G[0,0]:xxxx\r\n");
        }

        delete_membuf(buffer);
        delete_serial(serial);

        return 0;
}
