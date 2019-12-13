#include <rcom.h>

int control_panel_init(int argc, char **argv);
void control_panel_cleanup();

int control_panel_shutdown(void *userdata,
                           messagelink_t *link,
                           json_object_t command,
                           membuf_t *message);

int control_panel_display(void *userdata,
                          messagelink_t *link,
                          json_object_t command,
                          membuf_t *message);

void watchdog_onmessage(void *userdata,
                        messagelink_t *link,
                        json_object_t message);

void watchdog_onidle();
