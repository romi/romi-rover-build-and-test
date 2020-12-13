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
/**
 * Some basic vector calculations.
 * All operations are prefixed with 'mt' to avoid name clashes and get an
 * attempt for a unique prefix.
 *
 * @author Maurice Tollmien
 */

#include <math.h>
#include "vector.h"

/**
 * Creates a new vector from three given values.
 */
vector_t vector(double x, double y, double z)
{
    vector_t res;
    res.x = x;
    res.y = y;
    res.z = z;
    return res;
}

/**
 * Calculates the length of a given vector.
 */
double vector_length(vector_t vector)
{
        return sqrt((vector.x * vector.x)
                    + (vector.y * vector.y)
                    + (vector.z * vector.z));
}

/**
 * Normalises a vector and returnes a new, normalised one.
 */
vector_t vector_normalize(vector_t v)
{
    double l = vector_length(v);
    if (l >= .00001f)
        return vector(v.x / l, v.y / l, v.z / l);
    return v;
}

/**
 * Computes the cross-product of the vectors axb and returnes a new vector.
 */
vector_t vector_cross_product(vector_t a, vector_t b)
{
    vector_t product = vector((a.y * b.z - a.z * b.y),
                              (a.z * b.x - a.x * b.z),
                              (a.x * b.y - a.y * b.x));
    return product;
}

/**
 * Multiplies vector with scalar and returnes new vector.
 */
vector_t vector_mul_scalar(vector_t a, double s)
{
    vector_t res;
    res.x = a.x * s;
    res.y = a.y * s;
    res.z = a.z * s;
    return res;
}

/**
 * Calculates the scalar (outer) product of the given vectors.
 */
double vector_mul(vector_t a, vector_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/**
 * Subtracts vector b from vector a and returnes new vector.
 */
vector_t vector_sub(vector_t a, vector_t b)
{
    vector_t res;
    res.x = a.x - b.x;
    res.y = a.y - b.y;
    res.z = a.z - b.z;
    return res;
}

/**
 * Divides all values of the vector by s and returnes new vector.
 */
vector_t vector_div(vector_t a, double s)
{
    return vector_mul_scalar(a, 1.0/s);
}

/**
 * Adds two vectors and returns a new vector.
 */
vector_t vector_add(vector_t a, vector_t b)
{
    vector_t res;
    res.x = a.x + b.x;
    res.y = a.y + b.y;
    res.z = a.z + b.z;
    return res;
}

/**
 * Calculates the angle between two vectors.
 */
double vector_angle(vector_t a, vector_t b)
{
        double x = vector_mul(a, b) / (vector_length(a) * vector_length(b));
        if (x > 1.0) x = 1.0; 
        if (x < -1.0) x = -1.0; 
        return acos(x);
}

double vector_distance(vector_t a, vector_t b)
{
        double x = a.x - b.x;
        double y = a.y - b.y;
        return sqrt(x * x + y * y);
}
