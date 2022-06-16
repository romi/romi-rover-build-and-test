#ifndef _LIBROMI_RPI_GPIO_H_
#define _LIBROMI_RPI_GPIO_H_

#include "IGpio.h"
#include "GpioChip.h"
#include "GpioInput.h"
#include "GpioOutput.h"

namespace romi {

        class RpiGpio : public IGpio
        {
        protected:
                // See also https://pinout.xyz
                static const uint kOffsetPowerRelay = 27;    // Pin 13 
                static const uint kOffsetSecurityButton = 6; // Pin 31 

                GpioChip chip_;
                GpioOutput power_relay_;
                GpioInput security_button_;
        
        public:
                RpiGpio();
                virtual ~RpiGpio() override = default;

                bool set_power_relay(bool on) override;
                bool get_security_button(bool& on) override;
        };
}

#endif // _LIBROMI_RPI_GPIO_H_
