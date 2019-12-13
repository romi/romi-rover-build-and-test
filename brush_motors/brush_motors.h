#include <rcom.h>

int brush_motors_init(int argc, char **argv);
void brush_motors_cleanup();

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

void watchdog_onmessage(void *userdata,
                        messagelink_t *link,
                        json_object_t message);
