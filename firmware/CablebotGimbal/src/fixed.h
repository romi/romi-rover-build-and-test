#ifndef __FIXED_H
#define __FIXED_H

#include "Arduino.h"
#include <stdint.h>

#define kFractalNumBits 12
#define kFractalMask 0x00000fff 
#define kFractalMax 4096 
#define kFractalMaxFloat 4096.0f 
#define kIntegerNumBits 20
#define kIntegerMask 0xfffff000
#define kIntegerMaskShifted 0x000fffff

typedef int32_t fixed_t;

static inline fixed_t to_fixed(int32_t i)
{
        return (i & kIntegerMaskShifted) << kFractalNumBits;
}

static inline fixed_t to_fixed(int i)
{
        return to_fixed((int32_t) i);
}

static inline fixed_t to_fixed(float f)
{
        int32_t ii = (int32_t) truncf(f);
        float ff = f - (float) ii;
        int32_t fi = (int32_t) (ff * kFractalMaxFloat);
        return to_fixed(ii) + fi;
}

static inline fixed_t to_fixed(double f)
{
        return to_fixed((float) f);
}

static inline fixed_t fixed_add(fixed_t a, fixed_t b)
{
        return a + b;
}

static inline fixed_t fixed_sub(fixed_t a, fixed_t b)
{
        return a - b;
}

static inline fixed_t fixed_mul(fixed_t a, fixed_t b)
{
        int64_t r = a * b;
        r = (r >> kFractalNumBits) & 0x00000000ffffffff;
        return (fixed_t) r;
}

static inline int32_t fixed_int(fixed_t a)
{
        return (a >> kFractalNumBits);
}

static inline float fixed_frac(fixed_t a)
{
        int32_t fi = (a & kFractalMask);
        return (float) fi / kFractalMaxFloat;
}

static inline float fixed_float(fixed_t a)
{
        int32_t i = fixed_int(a);
        float fi = (float) i;
        float fr = fixed_frac(a);
        return fi + fr;
}

#endif // __FIXED_H
