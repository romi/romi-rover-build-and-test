
#ifndef _ZAZAI_AVERAGE_H
#define _ZAZAI_AVERAGE_H

#include "IFilter.h"

class Average : public IFilter
{
protected:

        static const uint8_t kSize = 5;
        
        uint8_t index_;
        double buffer_[kSize];
        double norm_;
        
public:
        
        Average()
                : index_(0),
                  norm_(1.0 / (double) kSize) {
                for (uint8_t i = 0; i < kSize; i++)
                        buffer_[i] = 0.0;
                
        }
        
        ~Average() override = default;

        double compute(double x) final {
                buffer_[index_] = x;
                if (++index_ == kSize)
                        index_ = 0;
                double average = 0.0;
                for (uint8_t i = 0; i < kSize; i++)
                        average += buffer_[i];
                return norm_ * average;
        }
};

#endif // _ZAZAI_AVERAGE_H
