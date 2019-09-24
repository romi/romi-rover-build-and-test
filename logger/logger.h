#include <rcom.h>

int logger_init(int argc, char **argv);
void logger_cleanup();
int logger_onmessage(void *userdata, messagelink_t *link, json_object_t message);  
int logger_listener_onmessage(void *userdata, messagelink_t *link, json_object_t message);  
