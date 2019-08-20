#include <rcom.h>

int interface_init(int argc, char **argv);  
void interface_cleanup();  

int interface_index(void *data, request_t *request);  
int interface_scripts(void *data, request_t *request);  
int interface_status(void *data, request_t *request);  
int interface_execute(void *data, request_t *request);  
int interface_registry(void *data, request_t *request);  
int interface_db(void *data, request_t *request);
int interface_listen(void *data, request_t *request);  
int interface_local_file(void *data, request_t *request);  


