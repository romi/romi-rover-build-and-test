#include <rcom.h>

int configuration_init(int argc, char **argv);
void configuration_cleanup();
void configuration_get(void *data, request_t *request, response_t *response);  
