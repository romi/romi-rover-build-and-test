#include <rcom.h>

int watchdog_init(int argc, char **argv);
void watchdog_cleanup();

int watchdog_shutdown(void *userdata,
                      messagelink_t *link,
                      json_object_t command,
                      membuf_t *message);

