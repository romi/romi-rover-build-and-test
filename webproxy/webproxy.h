#include <rcom.h>

int webproxy_init(int argc, char **argv);
void webproxy_cleanup();
int webproxy_get(void *data, request_t *request);

