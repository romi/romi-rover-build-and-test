/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
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

