
#ifndef _OQUAM_PLOTTER_H_
#define _OQUAM_PLOTTER_H_

#include "stepper.h"

#ifdef __cplusplus
extern "C" {
#endif

int print_paths(const char *filepath,
                list_t *paths,
                list_t *atdc_list,
                list_t *slices,
                block_t *blocks,
                int num_blocks,
                double *xmax,
                double *vmax,
                double *amax,
                double *scale);
        
#ifdef __cplusplus
}
#endif

#endif // _OQUAM_PLOTTER_H_
