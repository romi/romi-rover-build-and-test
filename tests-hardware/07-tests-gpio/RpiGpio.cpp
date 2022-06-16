#include <stdexcept>
#include "RpiGpio.h"

namespace romi {
        
        RpiGpio::RpiGpio()
                : chip_(),
                  power_relay_(chip_, kOffsetPowerRelay),
                  security_button_(chip_, kOffsetSecurityButton)
        {
        }

        bool RpiGpio::set_power_relay(bool value)
        {
                return power_relay_.set(value);
        }

        bool RpiGpio::get_security_button(bool& value)
        {
                return security_button_.get(value);
        }
}
