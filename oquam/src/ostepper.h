
#ifndef _OQUAM_OSTEPPER_H_
#define _OQUAM_OSTEPPER_H_

#include "controller.h"

#ifdef __cplusplus
extern "C" {
#endif

controller_t *new_oquam_stepper_controller(const char *device,
                                           double *xmax, double *vmax, double *amax,
                                           double *scale, double period);

#ifdef __cplusplus
}
#endif

#endif // _OQUAM_OSTEPPER_H_
