
#ifndef _ZAZAI_I_ARDUINO_UNO_H
#define _ZAZAI_I_ARDUINO_UNO_H

#include "IArduino.h"

class IArduinoUno : public IArduino
{
public:
        ~IArduinoUno() override = default;
        
        virtual uint32_t cpu_freq() = 0;
        virtual void block_interrupts() = 0;
        virtual void allow_interrupts() = 0;
        virtual void timer1_enable() = 0;
        virtual void timer1_disable() = 0;
        virtual void timer1_set_prescaling(uint16_t value) = 0;
        virtual void timer1_set_compare_value(uint16_t value) = 0;
        virtual void timer1_disconnect_output_pin() = 0;
        virtual void timer1_clear_timer_on_compare() = 0;
};

#endif // _ZAZAI_I_ARDUINO_UNO_H
