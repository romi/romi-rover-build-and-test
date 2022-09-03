#include <cstdlib>
#include <RSerial.h>
#include <Reader.h>
#include <util/Logger.h>
#include <Console.h>


using namespace romiserial;

int main()
{
        std::shared_ptr<ILog> log = std::make_shared<Console>();
        RSerial serial("/dev/serial0", 115200, 0, log);
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
