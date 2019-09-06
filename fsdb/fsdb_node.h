#include <rcom.h>

int fsdb_node_init(int argc, char **argv);
void fsdb_node_cleanup();
int fsdb_get(void *data, request_t *request);  
int fsdb_get_session(void *data, request_t *request);
int fsdb_onmessage(void *userdata, messagelink_t *link, json_object_t message);  
