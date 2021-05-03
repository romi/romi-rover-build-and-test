#ifndef _FIXED_H_
#define _FIXED_H_

#include <stdint.h>
#include <math.h>
#include <r.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t fixed_t; 
typedef int64_t double_fixed_t; 

/** The number of bits in the integer part (including sign) */
#define FIXED_WIDTH 32
#define FIXED_IBITS 8
#define FIXED_FBITS (FIXED_WIDTH-FIXED_IBITS)
#define FIXED_FMAX (1 << FIXED_FBITS)
#define FIXED_FMASK ((1 << FIXED_FBITS) - 1)

extern double fx_rounding;

void fx_init();

static inline
fixed_t itofx(int v) 
{
        fixed_t r = v;
	return (r << FIXED_FBITS);
}

static inline
int fxtoi(fixed_t v) 
{
	return (v >> FIXED_FBITS);
}

static inline
double fxtod(fixed_t r)
{
	/* return (double) (r >> FIXED_FBITS) + ((r & FIXED_FMASK) / (double) FIXED_FMAX); */
	fixed_t i = r >> FIXED_FBITS;
        double id = (double) i;
        fixed_t f =  (r & FIXED_FMASK);
        double fd = (double) f / (double) FIXED_FMAX;
        return id + fd;
}

static inline
fixed_t dtofx(double v) 
{
        r_debug("dtofx(%f)", v);
        int neg = (v < 0.0);
        if (neg) v = -v;
	fixed_t i = (fixed_t) v;
	fixed_t int_fx = (i << FIXED_FBITS);
	double frac_double =  v - i + fx_rounding;
	fixed_t frac_fx =  (fixed_t) (frac_double * FIXED_FMAX);
	fixed_t r = int_fx | frac_fx;
        return neg? -r : r;
}



static inline
fixed_t fxadd(fixed_t a, fixed_t b)
{
        return a + b;
}

static inline
fixed_t fxsub(fixed_t a, fixed_t b)
{
        return a - b;
}

static inline
fixed_t fxmul(fixed_t a, fixed_t b)
{
        double_fixed_t tmp = (double_fixed_t) a * (double_fixed_t) b;
        fixed_t r = (fixed_t) (tmp >> FIXED_FBITS);
        return r;
}

static inline
fixed_t fxrecip(fixed_t a)
{
        double d = fxtod(a);
        double r = 1.0 / d;
        return dtofx(r);
}

static inline
fixed_t fxdiv(fixed_t a, fixed_t b)
{
        return fxmul(a, fxrecip(b));
}

static inline
fixed_t fxexp(fixed_t a)
{
        double d = fxtod(a);
        double r = exp(d);
        return dtofx(r);
}

static inline
fixed_t fxsq(fixed_t a)
{
        return fxmul(a, a);
}

fixed_t fxrecip_1_to_0(fixed_t a);

#ifdef __cplusplus
}
#endif

#endif /* _FIXED_H_ */
