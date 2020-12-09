/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <math.h>
#include <romi.h>
#include "weeding.h"
#include "otsu.h"
#include "svm.h"
#include "quincunx.h"
#include "profiling.h"

////////////////////////////////////////////////////////////////////

image_t *segmentation_module_segment(segmentation_module_t *module,
                                     image_t *image,
                                     fileset_t *fileset,
                                     membuf_t *message)
{
        if (module->segment) {
                return module->segment(module, image, fileset, message);
        } else {
                r_warn("segmentation module has no compute function");
                return NULL;
        }
}

void delete_segmentation_module(segmentation_module_t *module)
{
        if (module->cleanup)
                module->cleanup(module);
}

list_t *path_module_compute(path_module_t *module,
                            image_t *image,
                            fileset_t *fileset,
                            membuf_t *message)
{
        if (module->compute) {
                return module->compute(module, image, fileset, message);
        } else {
                r_warn("path module has no compute function");
                return NULL;
        }
}

int path_module_set(path_module_t *module, const char *name,
                    json_object_t value, membuf_t *message)
{
        if (module->set) {
                return module->set(module, name, value, message);
        } else {
                r_warn("path module has no set function");
                membuf_printf(message, "path module has no set function");
                return -1;
        }
}

void delete_path_module(path_module_t *module)
{
        if (module->cleanup)
                module->cleanup(module);
}

////////////////////////////////////////////////////////////////////////

list_t *make_boustrophedon(double width, double height, double dx)
{
        list_t *path = NULL;
        double x, y, z;

        z = 0.0f;
        x = 0.0f;
        y = height;

        path = list_append(path, new_point(x, y, z));
        
        while (1) {
                y -= height;
                path = list_append(path, new_point(x, y, z));

                if (x + dx <= width) {
                        x += dx;
                        path = list_append(path, new_point(x, y, z));
                } else break;

                y += height;
                path = list_append(path, new_point(x, y, z));

                if (x + dx <= width) {
                        x += dx;
                        path = list_append(path, new_point(x, y, z));
                } else break;
        }

        z = 0.0f;
        path = list_append(path, new_point(x, y, z));

        return path;
}

/* list_t *compute_boustrophedon(workspace_t *workspace, double diameter_tool) */
/* { */
/*         return make_boustrophedon(workspace->width_meter, */
/*                                   workspace->height_meter, */
/*                                   diameter_tool);   */
/* } */

////////////////////////////////////////////////////////////////////////

image_t *get_workspace_view(fileset_t *fileset,
                            image_t *camera,
                            workspace_t *workspace,
                            int border)
{
        store_jpg(fileset, "camera", camera);
        
        /* image_t *rot = image_rotate(camera, */
        /*                             workspace->width, */
        /*                             camera->height - workspace->y0, */
        /*                             workspace->theta); */

        int x0 = workspace->x0 - border;
        int width = workspace->width + 2 * border;
        int y0 = camera->height - workspace->y0 - workspace->height - border;
        int height = workspace->height + 2 * border;

        if (x0 < 0) {
                r_err("get_workspace_view: camera position is not good: "
                      "the bottom is cut off");
                return NULL;
        }
        if (width > camera->width) {
                r_err("get_workspace_view: camera image width too small");
                return NULL;
        }
        if (y0 < 0) {
                r_err("get_workspace_view: camera position is not good: "
                      "the top is cut off");
                return NULL;
        }
        if (height > camera->height) {
                r_err("get_workspace_view: camera image height too small");
                return NULL;
        }
        
        image_t *cropped = FIXME_image_crop(/* rot */ camera, x0, y0, width, height);
        store_jpg(fileset, "cropped", cropped);

        //image_t *scaled = FIXME_image_scale(cropped, 4);
        image_t *scaled = FIXME_image_scale(cropped, 3);
        store_jpg(fileset, "scaled", scaled);

        /* delete_image(rot); */
        delete_image(cropped);

        return scaled;
}

////////////////////////////////////////////////////////////////////

pipeline_t *new_pipeline(workspace_t *workspace,
                         cnc_range_t *range,
                         segmentation_module_t *segmentation_module,
                         path_module_t *path_module)
{
        pipeline_t *p = r_new(pipeline_t);
        if (p == NULL)
                return NULL;
        p->workspace = workspace;
        p->cnc_range = range;
        p->segmentation_module = segmentation_module;
        p->path_module = path_module;
        return p;
}

void delete_pipeline(pipeline_t *p)
{ 
        if (p) {
                if (p->segmentation_module)
                        delete_segmentation_module(p->segmentation_module);
                if (p->path_module)
                        delete_path_module(p->path_module);
                r_delete(p);
        }
}

list_t *pipeline_run(pipeline_t *pipeline,
                     image_t *camera,
                     fileset_t *fileset,
                     membuf_t *message)
{
        image_t *image = NULL;
        image_t *mask = NULL;
        list_t *path = NULL;

        // FIXME: tool diameter hard_coded!!!
        double diameter_tool = 0.05;
        
        double width_meters = fabs( pipeline->cnc_range->x[1] -  pipeline->cnc_range->x[0]);
        double meters_to_pixels = pipeline->workspace->width / width_meters;
        double diameter = meters_to_pixels * diameter_tool;
        int border = (int) (diameter / 2.0);
        
        // 1. preprocessing: rotate, crop, and scale the camera image
        image = get_workspace_view(fileset, camera, pipeline->workspace, border);
        if (image == NULL)
                goto cleanup_and_exit;

        // 2. compute the binary mask (plants=white, soil=black...)
        mask = segmentation_module_segment(pipeline->segmentation_module,
                                           image, fileset, message);
        if (mask == NULL)
                goto cleanup_and_exit;

        // 3. compute the path
//        double meters_to_pixels = mask->width / pipeline->workspace->width_meter;
        path = path_module_compute(pipeline->path_module, mask, fileset, message);
        if (path == NULL)
                goto cleanup_and_exit;

        {
                membuf_t *path_buffer = new_membuf();
                for (list_t *l = path; l != NULL; l = list_next(l)) {
                        point_t *p = list_get(l, point_t);
                        membuf_printf(path_buffer, "%0.6f %0.6f\n", 3.0f * p->x, 3.0f * p->y);
                }                
                store_text(fileset, "path.txt", path_buffer);
                delete_membuf(path_buffer);
        }
        
        // 4. Change the coordinates from pixels to meters
        
        double ax = (pipeline->cnc_range->x[1] -  pipeline->cnc_range->x[0]) / mask->width;
        double bx = pipeline->cnc_range->x[0];
        double ay = (pipeline->cnc_range->y[1] -  pipeline->cnc_range->y[0]) / mask->height;
        double by = pipeline->cnc_range->y[0];
        
        for (list_t *l = path; l != NULL; l = list_next(l)) {
                point_t *p = list_get(l, point_t);
                p->y = mask->height - p->y;
                p->x = (float) (ax * p->x + bx);
                p->y = (float) (ay * p->y + by);
        }
        
        // Cleaning up
cleanup_and_exit:
        delete_image(image);
        delete_image(mask);

        // done
        return path;
}

int pipeline_set(pipeline_t *pipeline, const char *name,
                 json_object_t value, membuf_t *message)
{
        return path_module_set(pipeline->path_module, name, value, message);
}

