#include <rcom.h>

int video4linux_init(int argc, char **argv);
void video4linux_cleanup();
void video4linux_broadcast();
void video4linux_still(void *data, request_t *request, response_t *response);
