
#ifndef _ZAZAI_INCREMENTAL_ENCODER_H
#define _ZAZAI_INCREMENTAL_ENCODER_H

#include "IEncoder.h"

class IncrementalEncoder : public IEncoder
{
public:
        volatile int32_t position_;
        int32_t last_position_;
        int32_t increment_;
        // https://www.cuidevices.com/blog/what-is-encoder-ppr-cpr-and-lpr
        double pulses_per_revolution_;

        IncrementalEncoder()
                : position_(0),
                  last_position_(0),
                  increment_(1),
                  pulses_per_revolution_(1) {
        }
        
        ~IncrementalEncoder() override = default;
        
        void init(double pulses_per_revolution, int32_t increment) {
                pulses_per_revolution_ = pulses_per_revolution;
                increment_ = increment;
                position_ = 0;
        }

        int32_t get_position() override {
                return position_;
        }

        double get_speed(double dt) override {
                int32_t dp = position_ - last_position_;
                double speed = (double) dp / pulses_per_revolution_ / dt;
                last_position_ = position_;
                return speed;
        }
        
        inline void increment() {
                position_ += increment_;
        }
        
        inline void decrement() {
                position_ -= increment_;
        }

        // For testing only
        void set_position(int32_t t) {
                position_ = t;
        }
};

#endif // _ZAZAI_INCREMENTAL_ENCODER_H
