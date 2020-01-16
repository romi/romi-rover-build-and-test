
#ifndef _OQUAM_VIRT_H_
#define _OQUAM_VIRT_H_

#include "controller.h"

#ifdef __cplusplus
extern "C" {
#endif

controller_t *new_virtual_stepper_controller(const char *output,
                                             double *xmax, double *vmax, double *amax,
                                             double *scale, double period);

#ifdef __cplusplus
}
#endif

#endif // _OQUAM_VIRT_H_
