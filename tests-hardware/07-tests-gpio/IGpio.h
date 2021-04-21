#ifndef _LIBROMI_I_GPIO_H_
#define _LIBROMI_I_GPIO_H_

namespace romi {
        
        class IGpio // : public IEventSource
        {
        public:
                virtual ~IGpio() = default;
        
                virtual bool set_power_relay(bool on) = 0;
                virtual bool get_security_button(bool& on) = 0;

                //virtual int get_next_event() = 0;
        };
}

#endif // _LIBROMI_I_GPIO_H_
