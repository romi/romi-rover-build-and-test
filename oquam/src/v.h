
#ifndef _OQUAM_V_H_
#define _OQUAM_V_H_

#ifdef __cplusplus
extern "C" {
#endif

double *smul(double *w, double *v, double s);
double *sdiv(double *w, double *v, double s);

double *vadd(double *r, double *a, double *b);
double *vsub(double *r, double *a, double *b);
double *vmul(double *r, double *a, double *b);
double *vdiv(double *r, double *a, double *b);

double vmax(double *a);
double vmin(double *a);

double *vcopy(double *r, double *a);
double *vzero(double *r);
double *vset(double *r, double v);

double norm(double *v);
double *normalize(double *w, double *v);

int *vaddi(int *r, int *a, int *b);
int *vsubi(int *r, int *a, int *b);

int *vconvfi(int *r, double *a);
double *vconvif(double *r, int *a);

        
#ifdef __cplusplus
}
#endif

#endif // _OQUAM_V_H_
