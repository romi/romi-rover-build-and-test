#include <rcom.h>

int script_engine_init(int argc, char **argv);  
void script_engine_cleanup();  

void script_engine_scripts(void *data, request_t *request, response_t *response);  
void script_engine_status(void *data, request_t *request, response_t *response);  
void script_engine_execute(void *data, request_t *request, response_t *response);  


