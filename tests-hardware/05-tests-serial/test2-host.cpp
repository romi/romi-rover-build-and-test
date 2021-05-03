#include <stdlib.h>
#include <r.h>
#include <RomiSerialClient.h>
#include <RSerial.h>
#include <memory>
#include <iostream>

int main()
{
        std::shared_ptr<RSerial> serial = std::make_shared<RSerial>("/dev/serial0", 115200, 0);
        RomiSerialClient romi_serial(serial, serial);
        
        JsonCpp response;
        std::string str;
        
        while (true) {
                romi_serial.send("G", response);
                response.tostring(str, k_json_compact);
                std::cout << str << std::endl;
        }

        return 0;
}
