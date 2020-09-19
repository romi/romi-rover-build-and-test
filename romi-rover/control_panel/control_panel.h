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

void control_panel_onidle();
