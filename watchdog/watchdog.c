#include <stdlib.h>
#include <unistd.h>
#include "watchdog.h"

static void broadcast_log(void *userdata, const char* s)
{
        messagelink_t *get_messagelink_logger();
        messagelink_t *log = get_messagelink_logger();
        if (log)
                messagelink_send_str(log, s);
}

int watchdog_init(int argc, char **argv)
{ 
        r_log_set_writer(broadcast_log, NULL);
        return 0;
}

void watchdog_cleanup()
{
}

int watchdog_shutdown(void *userdata,
                      messagelink_t *link,
                      json_object_t command,
                      membuf_t *message)
{
	r_info("Shutting down");
        FILE *fp = popen("/sbin/poweroff", "r");
        while (!feof(fp) && !ferror(fp)) {
                char c;
                fread(&c, 1, 1, fp);
        }
        fclose(fp);
        return 0;
}

