
#ifndef _MOTORCONTROLLER_AVERAGE_H
#define _MOTORCONTROLLER_AVERAGE_H

#include "IFilter.h"

class Average : public IFilter
{
protected:

        static const uint8_t kSize = 5;
        
        uint8_t index_;
        int16_t buffer_[kSize];
        
public:
        
        Average() : index_(0) {
                for (uint8_t i = 0; i < kSize; i++)
                        buffer_[i] = 0.0;
                
        }
        
        ~Average() override = default;

        void push(int16_t x) {
                buffer_[index_] = x;
                if (++index_ == kSize)
                        index_ = 0;
        }

        int16_t get() {
                int16_t average = 0;
                for (uint8_t i = 0; i < kSize; i++)
                        average = (int16_t) (average + buffer_[i]);
                return (int16_t) (average / kSize);
        }
        
        int16_t compute(int16_t x) final {
                push(x);
                return get();
        }
};

#endif // _MOTORCONTROLLER_AVERAGE_H
