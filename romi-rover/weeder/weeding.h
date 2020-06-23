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
#ifndef _ROMI_WEEDING_H_
#define _ROMI_WEEDING_H_

#include <r.h>
#include <romi.h>
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *   The segmentation module takes an RGB image as input and computes
 *   a BW mask with white pixels for the crop and black pixels for the
 *   soil.
 */
typedef struct _segmentation_module_t segmentation_module_t;

typedef image_t *(*segmentation_module_compute_t)(segmentation_module_t *module,
                                                  image_t *image,
                                                  fileset_t *fileset,
                                                  membuf_t *message);

typedef void (*segmentation_module_cleanup_t)(segmentation_module_t *);

struct _segmentation_module_t {
        segmentation_module_compute_t segment;
        segmentation_module_cleanup_t cleanup;
};

image_t *segmentation_module_segment(segmentation_module_t *module,
                                     image_t *image,
                                     fileset_t *fileset,
                                     membuf_t *message);

void delete_segmentation_module(segmentation_module_t *module);


/**
 *   The path module takes a BW image and computes a path going over
 *   the black pixels.
 */

typedef struct _path_module_t path_module_t;

typedef list_t *(*path_module_compute_t)(path_module_t *module,
                                         image_t *image,
                                         fileset_t *fileset,
                                         membuf_t *message);

typedef int (*path_module_set_t)(path_module_t *module,
                                 const char *name,
                                 json_object_t value,
                                 membuf_t *message);

typedef void (*path_module_cleanup_t)(path_module_t *module);

struct _path_module_t {
        path_module_compute_t compute;
        path_module_cleanup_t cleanup;
        path_module_set_t set;
};

list_t *path_module_compute(path_module_t *module,
                            image_t *image,
                            fileset_t *fileset,
                            membuf_t *message);

void delete_path_module(path_module_t *module);

int path_module_set(path_module_t *module,
                    const char *name,
                    json_object_t value,
                    membuf_t *message);


/**
 *   The pipeline combines a segmentation and path module.
 */

typedef struct _pipeline_t {
        workspace_t *workspace;
        segmentation_module_t *segmentation_module;
        path_module_t *path_module;
} pipeline_t;

pipeline_t *new_pipeline(workspace_t *workspace,
                         segmentation_module_t *segmentation_module,
                         path_module_t *path_module);
void delete_pipeline(pipeline_t *pipeline);

list_t *pipeline_run(pipeline_t *pipeline,
                     image_t *image,
                     fileset_t *fileset,
                     membuf_t *message);

int pipeline_set(pipeline_t *pipeline,
                 const char *name,
                 json_object_t value,
                 membuf_t *message);

////

typedef struct _point_t {
        float x, y, z;
} point_t;

point_t *new_point(float x, float y, float z);
void delete_point(point_t *p);
void point_set(point_t *p, float x, float y, float z);

////

image_t *get_workspace_view(fileset_t *fileset, image_t *camera, workspace_t *workspace);
list_t *compute_boustrophedon(workspace_t *workspace, double diameter_tool);


        
#ifdef __cplusplus
}
#endif

#endif // _ROMI_WEEDING_H_
