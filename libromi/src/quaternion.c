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
/**
 * Implementation of most relevant functions on MTQuaternions.
 *
 * All operations are prefixed with 'mt' to avoid name clashes and get an
 * attempt for a unique prefix.
 *
 * @author Maurice Tollmien
 */

#include <math.h>

#include "romi/quaternion.h"
#include "romi/vector.h"

#define EPS 0.0001

/*
 * Low level operations on Quaternions
 */
quaternion_t quaternion_init(vector_t axis, double angle)
{
        quaternion_t q;
        q.s = cos(angle/2.0);
        q.v = vector_mul_scalar(axis, sin(angle/2.0));
        return q;
}

quaternion_t quaternion(double s, double x, double y, double z)
{
        quaternion_t q;
        q.s = s;
        q.v = vector(x, y, z);
        return q;
}

/**
 * Multiply to Quaternions with each other.
 * Careful! Not commutative!!!
 * Calculates: q1 * q2
 */
quaternion_t quaternion_mul(quaternion_t q1, quaternion_t q2)
{
        quaternion_t res;
        res.s = q1.s * q2.s - vector_mul(q1.v, q2.v);
        vector_t vres = vector_cross_product(q1.v, q2.v);
        vector_t tmp = vector_mul_scalar(q2.v, q1.s);
        vres = vector_add(vres, tmp);
        tmp = vector_mul_scalar(q1.v, q2.s);
        res.v = vector_add(vres, tmp);
        return res;
}

/**
 * Multiplies a Quaternion and a scalar.
 * Therefore the scalar will be converted to a Quaternion.
 * After that the two Quaternions will be muliplied.
 */
quaternion_t quaternion_mul_scalar(quaternion_t q, double s)
{
        quaternion_t q2;
        q2.s = s;
        q2.v = vector(0, 0, 0);
        return quaternion_mul(q, q2);
}

/**
 * Calculates: q1 + q2.
 */
quaternion_t quaternion_add(quaternion_t q1, quaternion_t q2)
{
        quaternion_t res;
        res.s = q1.s + q2.s;
        res.v = vector_add(q1.v, q2.v);
        return res;
}

/**
 * Calculates q1 - q2.
 */
quaternion_t quaternion_sub(quaternion_t q1, quaternion_t q2)
{
        quaternion_t res;
        res.s = q1.s - q2.s;
        res.v = vector_sub(q1.v, q2.v);
        return res;
}

/**
 * Complex conjugate the Quaternion.
 */
quaternion_t quaternion_conjugate(quaternion_t q)
{
        quaternion_t res;
        res.s = q.s;
        res.v.x = -q.v.x;
        res.v.y = -q.v.y;
        res.v.z = -q.v.z;
        return res;
}

/**
 * Invert the Quaternion.
 */
quaternion_t quaternion_inverse(quaternion_t q)
{
        double qlen = pow(quaternion_length(q), 2);
        quaternion_t tmp = quaternion_conjugate(q);
        return quaternion_mul_scalar(tmp, 1.0 / qlen);
}

/**
 * Normalize the Quaternion to a length of 1.
 */
quaternion_t quaternion_normalize(quaternion_t q)
{
        double qlen = quaternion_length(q);
        q.s /= qlen;
        q.v = vector_mul_scalar(q.v, 1.0 / qlen);
        return q;
}

/**
 * Calculates the length of the Quaternion.
 */
double quaternion_length(quaternion_t q)
{
        return sqrt(q.s * q.s + q.v.x * q.v.x + q.v.y * q.v.y + q.v.z * q.v.z);
}

/**
 * Check if the Quaternion is normalized.
 */
int quaternion_is_normalized(quaternion_t q)
{
        double res = q.s * q.s + q.v.x * q.v.x + q.v.y * q.v.y + q.v.z * q.v.z;
        return (res + EPS >= 1.0) && (res - EPS <= 1.0);
}

vector_t quaternion_rotate(quaternion_t q, vector_t point)
{
        q = quaternion_normalize(q);

        // Create Quaternion of the point to rotate
        quaternion_t p;
        p.s = 0.0;
        p.v = point;

        // The actual calculations.
        //  ---  q p q*  ---
        quaternion_t inverseQ = quaternion_inverse(q);
        quaternion_t res = quaternion_mul(q, p);
        res = quaternion_mul(res, inverseQ);

        // Write new rotated coordinates back to the point
        return res.v;
}

// From Wikipedia:
// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
// License: Creative Commons Attribution Share-Alike license (CC-BY-SA)?
vector_t convert_quaternion_to_euler(quaternion_t q)
{
	// roll (x-axis rotation)
	double sinr_cosp = +2.0 * (q.s * q.v.x + q.v.y * q.v.z);
	double cosr_cosp = +1.0 - 2.0 * (q.v.x * q.v.x + q.v.y * q.v.y);
	double roll = atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	double sinp = +2.0 * (q.s * q.v.y - q.v.z * q.v.x);
        double pitch;
	if (fabs(sinp) >= 1)
		pitch = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
	else
		pitch = asin(sinp);

	// yaw (z-axis rotation)
	double siny_cosp = +2.0 * (q.s * q.v.z + q.v.x * q.v.y);
	double cosy_cosp = +1.0 - 2.0 * (q.v.y * q.v.y + q.v.z * q.v.z);  
	double yaw = atan2(siny_cosp, cosy_cosp);

        vector_t res = { roll, pitch, yaw };
        return res;
}

// From Wikipedia:
// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
// License: Creative Commons Attribution Share-Alike license (CC-BY-SA)?
quaternion_t convert_euler_to_quaternion(double roll, double pitch, double yaw)
{
        // Abbreviations for the various angular functions
        double cy = cos(yaw * 0.5);
        double sy = sin(yaw * 0.5);
        double cr = cos(roll * 0.5);
        double sr = sin(roll * 0.5);
        double cp = cos(pitch * 0.5);
        double sp = sin(pitch * 0.5);

        quaternion_t q;
        q.s = cy * cr * cp + sy * sr * sp;
        q.v.x = cy * sr * cp - sy * cr * sp;
        q.v.y = cy * cr * sp + sy * sr * cp;
        q.v.z = sy * cr * cp - cy * sr * sp;
        return q;
}

