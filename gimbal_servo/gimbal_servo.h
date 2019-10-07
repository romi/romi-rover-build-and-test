#include <rcom.h>

int gimbal_servo_init(int argc, char **argv);
void gimbal_servo_cleanup();

int gimbal_onmoveat(void *userdata,
                    messagelink_t *link,
                    json_object_t command,
                    membuf_t *message);

int gimbal_onmoveto(void *userdata,
                    messagelink_t *link,
                    json_object_t command,
                    membuf_t *message);

int gimbal_onhoming(void *userdata,
                    messagelink_t *link,
                    json_object_t command,
                    membuf_t *message);
