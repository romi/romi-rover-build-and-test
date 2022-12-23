
#ifndef _ZAZAI_INCREMENTAL_ENCODER_UNO_H
#define _ZAZAI_INCREMENTAL_ENCODER_UNO_H

#include <Arduino.h>
#include "IncrementalEncoder.h"

typedef void (*EncoderInterruptHandler)();

class IncrementalEncoderUno : public IncrementalEncoder
{
public:
        uint8_t pin_b_;
        
        IncrementalEncoderUno() : IncrementalEncoder(), pin_b_(0) {
        }
        
        ~IncrementalEncoderUno() override = default;

        void init(double pulses_per_revolution,
                  int32_t increment, 
                  uint8_t pin_a, uint8_t pin_b,
                  EncoderInterruptHandler callback);
        
        inline void update() {
                bool b = digitalRead(pin_b_);
                if (b) {
                        decrement();
                } else {
                        increment();
                }
        }
        
};

#endif // _ZAZAI_INCREMENTAL_ENCODER_UNO_H
