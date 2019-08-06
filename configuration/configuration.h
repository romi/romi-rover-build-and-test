#include <rcom.h>

int configuration_init(int argc, char **argv);
void configuration_cleanup();
int configuration_get(void *data, request_t *request);  
