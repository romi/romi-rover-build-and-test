#include <stdlib.h>
#include <RSerial.h>
#include <Reader.h>
#include <r.h>

using namespace romiserial;

int main()
{
        RSerial serial("/dev/serial0", 115200, 0);
        Reader reader(serial);
        char buffer[64];
        
        while (true) {
                ssize_t n = reader.readline(buffer, sizeof(buffer));
                if (n >= 0) {
                        printf("%s\n", buffer);
                } else {
                        r_warn("reader.readline error");
                        break;
                }
        }

        return 0;
}
