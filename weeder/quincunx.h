#ifndef _ROMI_QUINCUNX_H_
#define _ROMI_QUINCUNX_H_

#include <r.h>
#include <romi.h>
#include "weeding.h"

#ifdef __cplusplus
extern "C" {
#endif

path_module_t *new_quincunx_module(double distance_plants,
                                   double distance_rows,
                                   double radius_zones,
                                   double diameter_tool,
                                   double threshold,
                                   double meters_to_pixels);

#ifdef __cplusplus
}
#endif

#endif // _ROMI_QUINCUNX_H_
