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
#ifndef _ROMI_PROFILING_H_
#define _ROMI_PROFILING_H_

#include <r.h>
#include <romi.h>
#include "weeding.h"

#ifdef __cplusplus
extern "C" {
#endif

fileset_t *create_fileset(scan_t *session, const char *fileset_id);

int store_workspace(fileset_t *fileset, workspace_t *workspace);
file_t *create_file(fileset_t *fileset, const char *name);
void store_jpg(fileset_t *fileset, const char *name, image_t *image);
void store_png(fileset_t *fileset, const char *name, image_t *image);
void store_text(fileset_t *fileset, const char *name, membuf_t *text);

        
#ifdef __cplusplus
}
#endif

#endif // _ROMI_PROFILING_H_
