#include <rcom.h>

int fake_camera_init(int argc, char **argv);
void fake_camera_cleanup();
void fake_camera_broadcast();
int fake_camera_still(void *data, request_t *request);  
