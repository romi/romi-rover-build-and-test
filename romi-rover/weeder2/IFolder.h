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

#ifndef __ROMI_I_FOLDER_H
#define __ROMI_I_FOLDER_H

#include <romi.h>
#include "Path.h"
#include "Image.h"

namespace romi {
        
        class IFolder
        {
        public:
                virtual ~IFolder() = default;
                
                virtual void store(const char* name, Image &image) = 0;
                virtual void store_jpg(const char* name, Image &image) = 0;
                virtual void store_png(const char* name, Image &image) = 0;

                
                virtual void store(const char* name, image_t *image) = 0;
                virtual void store_jpg(const char* name, image_t *image) = 0;
                virtual void store_png(const char* name, image_t *image) = 0;
                
                virtual void store_svg(const char* name, const char *body, int len) = 0;
                virtual void store_txt(const char* name, const char *body, int len) = 0;

                virtual void open_dump() = 0;
                virtual void dump(const char *name,
                                  int32_t rows, int32_t cols,
                                  float *values) = 0;
                virtual void dump(const char *name,
                                  int32_t rows, int32_t cols,
                                  double *values) = 0;
                virtual void dump_interleave(const char *name, int32_t size, 
                                             float *a, float *b) = 0;
                virtual void dump_interleave(const char *name, int32_t size,
                                             double *a, double *b) = 0;
                virtual void close_dump() = 0;

                virtual void print_path(float *x, float *y, int len, int n = -1) = 0;
                virtual void print_path(double *x, double *y, int len, int n = -1) = 0;
                virtual void print_path(Path &path, int n = -1) = 0;
                
        };
}

#endif // __ROMI_I_FOLDER_H
