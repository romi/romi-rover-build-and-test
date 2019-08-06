#include <rcom.h>

int fake_motors_init(int argc, char **argv);
void fake_motors_cleanup();

int motorcontroller_onmoveat(void *userdata,
                             messagelink_t *link,
                             json_object_t command,
                             membuf_t *message);

int motorcontroller_onenable(void *userdata,
                             messagelink_t *link,
                             json_object_t command,
                             membuf_t *message);

int motorcontroller_onreset(void *userdata,
                            messagelink_t *link,
                            json_object_t command,
                            membuf_t *message);

int motorcontroller_onhoming(void *userdata,
                             messagelink_t *link,
                             json_object_t command,
                             membuf_t *message);

void broadcast_encoders();
void broadcast_status();
