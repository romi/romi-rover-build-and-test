#include <stdlib.h>
#include <string.h>
#include <r.h>

int main()
{
        serial_t *serial = new_serial("/dev/serial0", 115200, 0);
        if (serial == nullptr) {
                r_err("Failed to open serial device");
                exit(1);
        }

        int counter = 0;
        char buffer[64];
        
        while (true) {
                snprintf(buffer, sizeof(buffer), "%d", counter++);
                int err = serial_println(serial, buffer);
                if (err != 0) {
                        r_warn("serial_println returned error");
                        break;
                }
        }

        delete_serial(serial);

        return 0;
}
