#include <stdexcept>
#include "RpiGpio.h"

namespace romi {
        
        RpiGpio::RpiGpio()
                : chip_(0),
                  power_relay_(chip_, kOffsetPowerRelay),
                  security_button_(chip_, kOffsetSecurityButton)
        {
        }

        void RpiGpio::set_power_relay(bool value)
        {
                power_relay_.set(value);
        }

        bool RpiGpio::get_security_button()
        {
                return security_button_.get();
        }
}
