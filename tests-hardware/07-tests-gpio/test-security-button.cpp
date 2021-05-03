#include <iostream>
#include <r.h>
#include "RpiGpio.h"

int main()
{
        romi::RpiGpio gpio;

        while (true) {
                bool on;
                gpio.get_security_button(on);
                if (on) 
                        std::cout << "ON" << std::endl;
                else
                        std::cout << "OFF" << std::endl;
        }
}
