#include <stdlib.h>
#include <string.h>
#include "Logger.h"
#include <RSerial.h>
#include <Printer.h>


using namespace romiserial;

int main()
{
        RSerial serial("/dev/serial0", 115200, 0);
        Printer printer(serial);
        
        int counter = 0;
        char buffer[64];
        
        while (true) {
                snprintf(buffer, sizeof(buffer), "%d", counter++);
                printf("%s\n", buffer);
                size_t n = printer.println(buffer);
                if (n != 1 + strlen(buffer)) {
                        r_warn("printer.println returned error");
                        break;
                }
        }

        return 0;
}
