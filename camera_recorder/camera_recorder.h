#include <rcom.h>

int camera_recorder_init(int argc, char **argv);
void camera_recorder_cleanup();

int camera_recorder_onstart(void *userdata,
                            messagelink_t *link,
                            json_object_t command,
                            membuf_t *message);

int camera_recorder_onstop(void *userdata,
                           messagelink_t *link,
                           json_object_t command,
                           membuf_t *message);

int camera_recorder_ondata(void *userdata,
                           const char *buf,
                           int len);

void camera_recorder_onpose(void *userdata,
                            datalink_t *link,
                            data_t *input,
                            data_t *output);
