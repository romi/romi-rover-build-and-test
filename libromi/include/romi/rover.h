#ifndef _LETTUCETHINK_ROVER_H_
#define _LETTUCETHINK_ROVER_H_

#include "vector.h"
#include "quaternion.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _rover_t rover_t;

rover_t *new_rover(double wheel_diameter, 
                   double wheel_base,
                   //double rover_length,
                   double encoder_steps);
void delete_rover(rover_t *r);

double rover_get_wheel_diameter(rover_t *r);
double rover_get_wheel_base(rover_t *r);
//double rover_get_rover_length(rover_t *r);

void rover_init_encoders(rover_t *r, double left, double right, double timestamp);
void rover_set_encoders(rover_t *r, double left, double right, double timestamp);
void rover_increment_encoders(rover_t *r, double left, double right, double timestamp);
void rover_set_location(rover_t *r, vector_t position);
void rover_set_orientation(rover_t *r, quaternion_t orientation);

vector_t rover_get_location(rover_t *r);
quaternion_t rover_get_orientation(rover_t *r);
vector_t rover_get_orientation_vector(rover_t *r);
void rover_set_angle(rover_t *r, double yaw);
void rover_get_pose(rover_t *r, vector_t *position, vector_t *speed, quaternion_t *orientation);


void rover_get_wheel_speeds(rover_t *r, double speed, double radius, double *left, double *right);
double rover_convert_distance(rover_t *r, double distance);

#ifdef __cplusplus
}
#endif

#endif // _LETTUCETHINK_ROVER_H_
