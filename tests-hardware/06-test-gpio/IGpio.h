#ifndef _LIBROMI_I_GPIO_H_
#define _LIBROMI_I_GPIO_H_

namespace romi {
        
        class IGpio // : public IEventSource
        {
        public:
                virtual ~IGpio() = default;
        
                virtual void set_power_relay(bool value) = 0;
                virtual bool get_security_button() = 0;

                //virtual int get_next_event() = 0;
        };
}

#endif // _LIBROMI_I_GPIO_H_
