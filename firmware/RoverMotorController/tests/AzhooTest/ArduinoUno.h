
#ifndef _ZAZAI_ARDUINO_UNO_H
#define _ZAZAI_ARDUINO_UNO_H

#include <Arduino.h>
#include "IArduinoUno.h"
#include "Timer1.h"
#include "IncrementalEncoder.h"

class ArduinoUno : public IArduinoUno
{
protected:
        Timer1 timer1_;
        IncrementalEncoder left_encoder_;
        IncrementalEncoder right_encoder_;
        
public:
        
        static ArduinoUno& get();
        
        ArduinoUno() : timer1_(*this) {}
        ~ArduinoUno() override = default;

        ITimer& timer1() override {
                return timer1_;
        }
        
        bool pin_is_high(int id) final {
                return digitalRead(id);
        }

        void block_interrupts() override;
        void allow_interrupts() override;
        uint32_t cpu_freq() override;        
        void timer1_enable() override;
        void timer1_disable() override;
        void timer1_set_prescaling(uint16_t value) override;
        void timer1_set_compare_value(uint16_t value) override;
        void timer1_disconnect_output_pin() override;
        void timer1_clear_timer_on_compare() override;

        // static void timer1_interrupt_handler();
};

#endif // _ZAZAI_ARDUINO_UNO_H
