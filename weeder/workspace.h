#ifndef _ROMI_WORKSPACE_H_
#define _ROMI_WORKSPACE_H_

#include <r.h>
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
        double theta;
        int x0;
        int y0;
        int width;
        int height;
        double width_meter;
        double height_meter;
} workspace_t;

int workspace_parse(workspace_t *workspace, json_object_t obj);
json_object_t workspace_to_json(workspace_t *workspace);
        
#ifdef __cplusplus
}
#endif

#endif // _ROMI_WORKSPACE_H_
