#include <rcom.h>

int interface_init(int argc, char **argv);  
void interface_cleanup();  

void interface_index(void *data, request_t *request, response_t *response);  
void interface_scripts(void *data, request_t *request, response_t *response);  
void interface_status(void *data, request_t *request, response_t *response);  
void interface_execute(void *data, request_t *request, response_t *response);  
void interface_registry(void *data, request_t *request, response_t *response);  
void interface_db(void *data, request_t *request, response_t *response);
void interface_local_file(void *data, request_t *request, response_t *response);  


