#ifndef _ROMI_WEEDING_H_
#define _ROMI_WEEDING_H_

#include <romi.h>

#ifdef __cplusplus
extern "C" {
#endif


////////////////////////////////////////////////////////////////////

// (x0,y0) are the pixel coordinates of the origin of the CNC in the
// topcam image.
//
// theta is the angle, in degrees, over which the image should rotated
// (around x0,y0) to align the topcam image withe frame of the rover
// topcam image. Only a rotation around the z-axis is considered for
// now.
//
// width_px and height_px are the width and height of the workspace,
// measured in pixels, as seen in the topcam image.
// 
// width_mm, height_mm are the width and the height of the real
// workspace, measured in millimeters.

typedef struct _workspace_t {
        float theta;
        int x0;
        int y0;
        int width;
        int height;
        int width_mm;
        int height_mm;
} workspace_t;

////

typedef struct _point_t {
        float x, y, z;
} point_t;

point_t *new_point(float x, float y, float z);
void delete_point(point_t *p);
void point_set(point_t *p, float x, float y, float z);

////

list_t *compute_path(image_t *camera, workspace_t *workspace,
                     float distance, float z0, float threshold);
list_t *compute_boustrophedon(workspace_t *workspace, float z0);

////

int set_weeding_data_directory(const char *directory);
void free_weeding_data_directory();

        
#ifdef __cplusplus
}
#endif

#endif // _ROMI_WEEDING_H_
