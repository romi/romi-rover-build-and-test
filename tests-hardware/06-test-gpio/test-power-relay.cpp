#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <r.h>

#include "RpiGpio.h"

int main()
{
        romi::RpiGpio gpio;

        while (true) {
                gpio.set_power_relay(true);
                clock_sleep(1.0);
                gpio.set_power_relay(false);
                clock_sleep(1.0);
        }
}
