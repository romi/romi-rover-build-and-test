/*
  This file is based on Maurice Tollmien's original Vector Library,
  https://github.com/MauriceGit/Vector_Library. The original copyright
  statement can be found below.

 */

/*
Copyright (c) 2015, 2016 Maurice Tollmien <maurice.tollmien@gmail.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
#ifndef __ROMI_VECTOR_H__
#define __ROMI_VECTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Interface for basic vector calculations.
 * All operations are prefixed with 'mt' to avoid name clashes and get an
 * attempt for a unique prefix.
 *
 * @author Maurice Tollmien
 */

typedef struct {
    double x;
    double y;
    double z;
} vector_t;

#define MT_PI 3.14159265

vector_t vector(double x, double y, double z);
double vector_length(vector_t vector);
vector_t vector_normalize(vector_t vector);
vector_t vector_cross_product(vector_t a, vector_t b);
double vector_distance(vector_t a, vector_t b);
double vector_mul(vector_t a, vector_t b);
vector_t vector_sub(vector_t a, vector_t b);
vector_t vector_add(vector_t a, vector_t b);
vector_t vector_div(vector_t a, double s);
vector_t vector_mul_scalar(vector_t a, double s);
double vector_angle(vector_t a, vector_t b);

#ifdef __cplusplus
}
#endif

#endif
