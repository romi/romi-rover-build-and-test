
// for log_xxx()
#include <rcom.h>

#include <string.h>
#include <math.h>

#include "romi/mem.h"
#include "romi/rover.h"


struct _rover_t {
        // The current location and orientation
        vector_t location;
        vector_t speed;
        quaternion_t orientation;
        double encoder[2];
        int initialized;
        double last_timestamp;
        
        // The displacement, in meters, and the change in orientation
        // (only tracking the change in yaw) relative to the 'current'
        // location;
        vector_t displacement;
        double theta;
                
        double wheel_diameter; 
        double wheel_circumference; 
        double wheel_base;
        //double rover_length;
        double encoder_steps;
};

static void rover_update_pose(rover_t *r,
                              double left, double right,
                              double timestamp);

rover_t *new_rover(double wheel_diameter, 
                   double wheel_base,
                   //double rover_length,
                   double encoder_steps)
{
        rover_t *r = new_obj(rover_t);
        if (r == NULL) return NULL;
        memset(r, 0, sizeof(rover_t));

        r->wheel_diameter = wheel_diameter;
        r->wheel_circumference = M_PI * wheel_diameter;
        r->wheel_base = wheel_base;
        //r->rover_length = rover_length;
        r->encoder_steps = encoder_steps;
        r->encoder[0] = 0.0;
        r->encoder[1] = 0.0;
        r->initialized = 0;
        r->orientation = quaternion(1, 0, 0, 0);

        return r;
}

void delete_rover(rover_t *r)
{
        if (r) {
                delete_obj(r);
        }
}

double rover_get_wheel_diameter(rover_t *r)
{
        return r->wheel_diameter;
}

double rover_get_wheel_base(rover_t *r)
{
        return r->wheel_base;
}

/* double rover_get_rover_length(rover_t *r) */
/* { */
/*         return r->rover_length; */
/* } */

void rover_init_encoders(rover_t *r, double left, double right, double timestamp)
{
        r->encoder[0] = left;
        r->encoder[1] = right;
        r->last_timestamp = timestamp;
}

void rover_set_encoders(rover_t *r, double left, double right, double timestamp)
{
        // Update the estimate of the rover location and
        // orientation based on the wheel odometry.
        rover_update_pose(r, left, right, timestamp);
}

void rover_increment_encoders(rover_t *r, double left, double right, double timestamp)
{
        // Update the estimate of the rover location and
        // orientation based on the wheel odometry.
        rover_update_pose(r, r->encoder[0] + left, r->encoder[1] + right, timestamp);
}

void rover_set_location(rover_t *r, vector_t location)
{
        r->location = location;
        // reinitialise the incremental displacement to zero
        r->displacement.x = r->displacement.y = r->displacement.z = 0;
}

void rover_set_angle(rover_t *r, double yaw)
{
        r->orientation = convert_euler_to_quaternion(0, 0, yaw);
        // reinitlialise the incremental angle
        r->theta = 0.0;
}

void rover_set_orientation(rover_t *r, quaternion_t orientation)
{
        r->orientation = orientation;
        // reinitlialise the incremental angle
        r->theta = 0.0;
}

vector_t rover_get_location(rover_t *r)
{
        return vector_add(r->location, r->displacement);
}

quaternion_t rover_get_orientation(rover_t *r)
{
        quaternion_t dtheta = convert_euler_to_quaternion(0, 0, r->theta);
        return quaternion_mul(dtheta, r->orientation);
}

vector_t rover_get_orientation_vector(rover_t *r)
{
        quaternion_t q = rover_get_orientation(r);
        vector_t v = vector(1, 0, 0);
        return quaternion_rotate(q, v);
}

void rover_get_pose(rover_t *r, vector_t *location, vector_t *speed, quaternion_t *orientation)
{
        if (orientation) {
                quaternion_t dtheta = convert_euler_to_quaternion(0, 0, r->theta);
                *orientation = quaternion_mul(dtheta, r->orientation);
                //*orientation = dtheta;
        }
        if (speed)
                *speed = r->speed;
        if (location)
                *location = vector_add(r->location, r->displacement);
}

static void rover_update_pose(rover_t *r,
                              double left, double right,
                              double timestamp)
{
        double dx, dy;
        double dL, dR;
        double half_wheel_base = 0.5 * r->wheel_base;
        double alpha;

        if (!r->initialized) {
                r->encoder[0] = left;
                r->encoder[1] = right;
                r->initialized = 1;
                return;
        }

        /* log_debug("encL %f, encR %f steps", left, right); */
        
        // dL and dR are the distances travelled by the left and right
        // wheel.
        dL = left - r->encoder[0];
        dR = right - r->encoder[1];
        log_debug("dL %f, dR %f steps", dL, dR);
        
        dL = r->wheel_circumference * dL / r->encoder_steps;
        dR = r->wheel_circumference * dR / r->encoder_steps;
        log_debug("dL %f, dR %f m", dL, dR);

        // dx and dy are the changes in the location of the rover, in
        // the frame of reference of the rover.
        if (dL == dR) {
                dx = dL;
                dy = 0.0;
                alpha = 0.0;
        } else {
                double radius = 0.5 * r->wheel_base * (dL + dR) / (dR - dL);
                /* log_debug("radius %f", radius); */
                if (radius >= 0) {
                        alpha = dR / (radius + half_wheel_base);
                } else {
                        alpha = -dL / (-radius + half_wheel_base);
                }
                dx = radius * sin(alpha);
                dy = radius - radius * cos(alpha);
        }

        /* log_debug("dx %f, dy %f, alpha %f", dx, dy, alpha); */

        // Convert dx and dy to the changes in the last frame of
        // reference (i.e. relative to the current orientation).
        double c = cos(r->theta);
        double s = sin(r->theta);
        double dx_ = c * dx - s * dy;
        double dy_ = s * dx + c * dy;

        /* log_debug("dx_ %f, dy_ %f", dx_, dy_); */

        r->displacement.x += dx_;
        r->displacement.y += dy_;
        r->theta += alpha;
        r->encoder[0] = left;
        r->encoder[1] = right;

        if (r->last_timestamp == 0.0) {
                r->last_timestamp = timestamp;
        } else {
                double dt = timestamp - r->last_timestamp;
                if (dt != 0.0) {
                        double vx = dx_ / dt;
                        double vy = dy_ / dt;
                        r->speed.x = 0.8 * r->speed.x + 0.2 * vx;
                        r->speed.y = 0.8 * r->speed.y + 0.2 * vy;
                        r->last_timestamp = timestamp;
                }
        }
        
        //log_debug("displacement:  %f %f - angle %f",
        // r->displacement.x, r->displacement.y, r->theta * 180.0 / M_PI);
        //log_debug("speed:  %f %f", r->speed.x, r->speed.y);

        //quaternion_t dtheta = convert_euler_to_quaternion(0, 0, r->theta);
        //*orientation = MultQuaternionQuaternion(&dtheta, &r->orientation);
        //log_debug("quaternion: %f %f %f %f", dtheta.s, dtheta.v.x, dtheta.v.y, dtheta.v.z);
}

void rover_get_wheel_speeds(rover_t *r, double speed, double radius, double *left, double *right)
{
        if (radius == 0.0) {
                *left = 0.0;
                *right = 0.0;
        } else {
                *left = speed * (radius - 0.5 * r->wheel_base) / radius;
                *right = speed * (radius + 0.5 * r->wheel_base) / radius;
        }
}

double rover_convert_distance(rover_t *r, double distance)
{
        return r->encoder_steps * distance / r->wheel_circumference;
}
