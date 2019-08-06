/*
  This file is based on Maurice Tollmien's original Quaternion
  Library, https://github.com/MauriceGit/Quaternion_Library. The
  original copyright statement can be found below.

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
#ifndef _ROMI_QUATERNION_H_
#define _ROMI_QUATERNION_H_

/**
 * Interface for some operations on Quaternions.
 * All operations are prefixed with 'mt' to avoid name clashes and get an
 * attempt for a unique prefix.
 *
 * @author Maurice Tollmien
 */

#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

/** quaternion_t */
typedef struct {
        double s;
        vector_t v;
} quaternion_t;

quaternion_t quaternion_init(vector_t axis, double angle);
quaternion_t quaternion(double s, double x, double y, double z);

quaternion_t quaternion_conjugate(quaternion_t q);
quaternion_t quaternion_inverse(quaternion_t q);
quaternion_t quaternion_normalize(quaternion_t q);
int quaternion_is_normalized(quaternion_t q);
double quaternion_length(quaternion_t q);

quaternion_t quaternion_mul(quaternion_t q1, quaternion_t q2);
quaternion_t quaternion_add(quaternion_t q1, quaternion_t q2);
quaternion_t quaternion_sub(quaternion_t q1, quaternion_t q2);
quaternion_t quaternion_mul_scalar(quaternion_t q, double s);

/* Some higher level functions, using Quaternions */
vector_t quaternion_rotate(quaternion_t q, vector_t point);

// Added (PH, 20181119)
vector_t convert_quaternion_to_euler(quaternion_t q);
quaternion_t convert_euler_to_quaternion(double roll, double pitch, double yaw);

#ifdef __cplusplus
}
#endif

#endif // _ROMI_QUATERNION_H_

