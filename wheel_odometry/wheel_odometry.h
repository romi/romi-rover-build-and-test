#include <rcom.h>

int wheel_odometry_init(int argc, char **argv);
void wheel_odometry_cleanup();

void wheel_odometry_ondata(void *userdata, datalink_t *link,
                           data_t *input, data_t *output);
