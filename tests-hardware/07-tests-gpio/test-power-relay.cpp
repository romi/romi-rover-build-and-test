#include <ClockAccessor.h>
#include "RpiGpio.h"

int main()
{
        romi::RpiGpio gpio;

        while (true) {
                gpio.set_power_relay(true);
                rpp::ClockAccessor::GetInstance()->sleep(1.0);
                gpio.set_power_relay(false);
                rpp::ClockAccessor::GetInstance()->sleep(1.0);
        }
}
