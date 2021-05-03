
#include "som/fixed.h"

double fx_rounding = 0.0;
static fixed_t fx_reciptab[256];

static void fx_init_reciptab()
{
        int i;
        fx_reciptab[0] = 0;
        for (i = 1; i < 256; i++) {
                fixed_t a = (i << (FIXED_FBITS - 8));
                double x = fxtod(a);
                double recip = 1.0 / x;
                r_debug("fx_init_reciptab: x=%f, 1/x=%f", x, recip);
                fixed_t b = dtofx(recip); 
                fx_reciptab[i] = b;
        }
}

void fx_init()
{
        fx_rounding = pow(2.0, -FIXED_FBITS-1);        
        fx_init_reciptab();
}

fixed_t fxrecip_1_to_0(fixed_t a)
{
        
#if 0
        // Without interpolation
        static fixed_t rounding_mask = 1 << (FIXED_FBITS - 9);
        int i = ((a >> (FIXED_FBITS - 8)) & 0xff); // take 8 significant bits
        if (a & rounding_mask) i++;
        return fx_reciptab[i];

#else
        // With interpolation
        static fixed_t fraction_mask = ((1 << (FIXED_FBITS - 8)) - 1);
        static fixed_t one = (1 << FIXED_FBITS);

        int i = ((a >> (FIXED_FBITS - 8)) & 0xff); // take 8 significant bits

        fixed_t fraction1 = (a & fraction_mask) << 8;
        fixed_t fraction2 = one - fraction1;
        fixed_t r = fx_reciptab[i];
        fixed_t r1 = fxmul(r, fraction2);
        r = fx_reciptab[i+1];
        fixed_t r2 = fxmul(r, fraction1);
        r = r1 + r2;

        return r;
#endif
}

static inline
fixed_t fxrecip_2_to_0(fixed_t a)
{
        a = a >> 1; // divide by 2
        fixed_t r = fxrecip_1_to_0(a);
        r = r >> 1; // divide by 2
        return r;
}
