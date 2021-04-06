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
                const char *counter = serial_readline(serial, buffer);
                if (counter != nullptr) {
                        r_info("%s", counter);
                } else {
                        r_warn("serial_readline returned null");
                        break;
                }
        }

        delete_membuf(buffer);
        delete_serial(serial);

        return 0;
}
