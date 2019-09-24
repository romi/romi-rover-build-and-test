#include "logger.h"

messagehub_t *get_messagehub_log();

int logger_init(int argc, char **argv)
{
        return 0;
}

void logger_cleanup()
{
}

int logger_onmessage(void *userdata, messagelink_t *link, json_object_t message)
{
        messagehub_t *hub = get_messagehub_log();
        messagehub_broadcast_obj(hub, NULL, message);
        return 0;
}
