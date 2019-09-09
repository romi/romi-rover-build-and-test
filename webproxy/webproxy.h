#include <rcom.h>

int webproxy_init(int argc, char **argv);
void webproxy_cleanup();
int webproxy_onconnect(void *userdata, messagehub_t *hub,
                       request_t* request, messagelink_t *link);

void webproxy_onrequest(void *data,
                        request_t *request,
                        response_t *response);

