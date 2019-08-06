#include <rcom.h>

int navigation_init(int argc, char **argv);
void navigation_cleanup();

void navigation_onpose(void *userdata,
                       datalink_t *link,
                       data_t *input,
                       data_t *output);

int navigation_onmove(void *userdata,
                      messagelink_t *link,
                      json_object_t command,
                      membuf_t *message);
