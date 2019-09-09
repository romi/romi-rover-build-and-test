#include <rcom.h>

int fsdb_node_init(int argc, char **argv);
void fsdb_node_cleanup();
void fsdb_get(void *data, request_t *request, response_t *response);  
void fsdb_get_directory(void *data, request_t *request, response_t *response);
int fsdb_onmessage(void *userdata, messagelink_t *link, json_object_t message);  
