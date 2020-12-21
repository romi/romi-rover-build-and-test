/*
  libromi

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  Libromi provides common abstractions and functions for ROMI
  applications.

  libromi is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#ifndef _ROMI_IMAGE_H_
#define _ROMI_IMAGE_H_

#include <r.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
        IMAGE_BW,
        IMAGE_RGB,
        IMAGE_HSV
}; 

typedef struct _image_t {
        int width;
        int height;
        int type;
        int channels;
        float *data;
} image_t;

image_t *new_image(int type, int width, int height);
image_t *new_image_bw(int width, int height);
image_t *new_image_rgb(int width, int height);
image_t *new_image_hsv(int width, int height);
void delete_image(image_t *image);

image_t *image_clone(image_t *im);

image_t *image_load(const char *filename);
image_t *image_load_from_mem(const unsigned char *data, int len);

int image_store(image_t* image, const char *filename, const char *type);
int image_store_to_mem(image_t* image, membuf_t *out, const char *format);

const char *image_type(const char *filename);
const char *image_mimetype(const char *format);

int image_contains(image_t *image, int x, int y);


#define image_contains(__im, __x, __y) \
        ((__x) >= 0 && (__x) < (__im)->width && (__y) >= 0 && (__y) < (__im)->height)

#define image_set(__im, __x, __y, __chan, __clr)                        \
        if (image_contains((__im), (__x), (__y)))                       \
                (__im)->data[(__im)->channels * ((__y) * (__im)->width + (__x)) + (__chan)] = __clr

#define image_get(__im, __x, __y, __chan)                               \
        (image_contains((__im), (__x), (__y))?                          \
         (__im)->data[(int)((__im)->channels * ((__y) * (__im)->width + (__x)) + (__chan))] : 0.0f)


int image_width(image_t *image);
int image_height(image_t *image);
void image_clear(image_t *image);
void image_fill(image_t *image, int channel, float color);
void image_offset(image_t *image, float offset, int channel);
void image_circle(image_t *image, float xc, float yc, float radius, float* color);
void image_bell(image_t *image, float xc, float yc, float stddev);
image_t *image_binary(image_t* image, float threshold);
image_t *image_in_range(image_t* image, float *min, float *max);
void image_range_stats(image_t* image, float *min, float *max,
                       int32_t *count, float *cx, float *cy);
image_t *image_rotate(image_t *image, float xc, float yc, double radians);
image_t *image_convert_bw(image_t *image);
image_t *image_convert_hsv(image_t *image);
image_t *image_convert_rgb(image_t *image);

// FIXME: check boundaries! This function supposes that the rectangle
// fits in the original image. Beware!
image_t *FIXME_image_crop(image_t *image, int x, int y, int width, int height);

// FIXME: should do a low-pass filter, but it doesn't
image_t *FIXME_image_scale(image_t *image, int n);
image_t *image_scale(image_t *image, int width, int height);

int image_split_rgb(image_t *rgb_in, image_t **rgb_out);

image_t* image_excess_green(image_t* image);


int convert_to_jpeg(uint8_t* rgb, int width, int height, int quality, membuf_t *out);
void image_import(image_t *image, uint8_t* rgb);
image_t *convert_to_image(uint8_t* rgb, int width, int height);

        
#ifdef __cplusplus
}
#endif

#endif // _ROMI_IMAGE_H_
