#include <math.h>
#include "v.h"

#define N 3

double norm(double *v)
{
       return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

double *vabs(double *r, double *a)
{
        r[0] = fabs(a[0]);
        r[1] = fabs(a[1]);
        r[2] = fabs(a[2]);
        return r;
}

double *vcopy(double *r, double *a)
{
        r[0] = a[0];
        r[1] = a[1];
        r[2] = a[2];
        return r;
}

double *vzero(double *r)
{
        r[0] = 0.0;
        r[1] = 0.0;
        r[2] = 0.0;
        return r;
}

double *vset(double *r, double v)
{
        r[0] = v;
        r[1] = v;
        r[2] = v;
        return r;
}

double *smul(double *w, double *v, double s)
{
        w[0] = v[0] * s;
        w[1] = v[1] * s;
        w[2] = v[2] * s;
        return w;
}

double *sdiv(double *w, double *v, double s)
{
        w[0] = v[0] / s;
        w[1] = v[1] / s;
        w[2] = v[2] / s;
        return w;
}

double vmax(double *a)
{
        double r = a[0];
        if (r < a[1])
                r = a[1];
        if (r < a[2])
                r = a[2];
        return r;
}

double vmin(double *a)
{
        double r = a[0];
        if (r > a[1])
                r = a[1];
        if (r > a[2])
                r = a[2];
        return r;
}

double *vadd(double *r, double *a, double *b)
{
        r[0] = a[0] + b[0];
        r[1] = a[1] + b[1];
        r[2] = a[2] + b[2];
        return r;
}

double *vsub(double *r, double *a, double *b)
{
        r[0] = a[0] - b[0];
        r[1] = a[1] - b[1];
        r[2] = a[2] - b[2];
        return r;
}

double *vmul(double *r, double *a, double *b)
{
        r[0] = a[0] * b[0];
        r[1] = a[1] * b[1];
        r[2] = a[2] * b[2];
        return r;
}

double *vdiv(double *r, double *a, double *b)
{
        r[0] = a[0] / b[0];
        r[1] = a[1] / b[1];
        r[2] = a[2] / b[2];
        return r;
}

double *normalize(double *w, double *v)
{
        double L = norm(v);
        w[0] = v[0] / L;
        w[1] = v[1] / L;
        w[2] = v[2] / L;
        return w;
}

int *vaddi(int *r, int *a, int *b)
{
        r[0] = a[0] + b[0];
        r[1] = a[1] + b[1];
        r[2] = a[2] + b[2];
        return r;
}

int *vsubi(int *r, int *a, int *b)
{
        r[0] = a[0] - b[0];
        r[1] = a[1] - b[1];
        r[2] = a[2] - b[2];
        return r;
}

int *vconvfi(int *r, double *a)
{
        r[0] = (int) a[0];
        r[1] = (int) a[1];
        r[2] = (int) a[2];
        return r;
}

double *vconvif(double *r, int *a)
{
        r[0] = (double) a[0];
        r[1] = (double) a[1];
        r[2] = (double) a[2];
        return r;
}
