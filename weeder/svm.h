#ifndef _ROMI_SVM_H_
#define _ROMI_SVM_H_

#include <r.h>
#include <romi.h>
#include "weeding.h"

#ifdef __cplusplus
extern "C" {
#endif

segmentation_module_t *new_svm_module(float *a, float b);
        
#ifdef __cplusplus
}
#endif

#endif // _ROMI_SVM_H_
