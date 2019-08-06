#include <rcom.h>

int weeder_init(int argc, char **argv);
void weeder_cleanup();

int weeder_onhoe(void *userdata,
                 messagelink_t *link,
                 json_object_t command,
                 membuf_t *message);
