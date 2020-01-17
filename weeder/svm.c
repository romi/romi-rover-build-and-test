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
#include "svm.h"

typedef struct _svm_module_t {
        segmentation_module_t interface;
        float a[3];
        float b;
} svm_module_t;

static void delete_svm_module(segmentation_module_t *module);

static image_t *svm_segmentation_function(segmentation_module_t *m,
                                          image_t *in,
                                          fileset_t *fileset,
                                          membuf_t *message)
{
        svm_module_t *module = (svm_module_t *) m; 
 
        if (in->type != IMAGE_RGB) {
                r_err("Expected an RGB input image");
                return NULL;
        }

        image_t *out = new_image(IMAGE_BW, image_width(in), image_height(in));
        if (out == NULL)
                return NULL;
        
        /* for (int i = 0; i < 3; i++) */
        /*         a[i] *= 256.0f; */

        int len = image_width(in) * image_height(in);
        for (int i = 0, j = 0; i < len; i++, j += 3) {
                float x = (in->data[j] * module->a[0]
                           + in->data[j+1] * module->a[1]
                           + in->data[j+2] * module->a[2]
                           + module->b);
                out->data[i] = (x > 0.0f)? 1.0 : 0.0;
        }
        
        return out;
}

segmentation_module_t *new_svm_module(float *a, float b)
{
        svm_module_t *module; 
        module = r_new(svm_module_t);
        if (module == NULL)
                return NULL;
        module->interface.segment = svm_segmentation_function;
        module->interface.cleanup = delete_svm_module;
        for (int i = 0; i < 3; i++)
                module->a[i] = a[i];
        module->b = b;
        return (segmentation_module_t *) module;
}

void delete_svm_module(segmentation_module_t *module)
{
        if (module)
                r_delete(module);
}
