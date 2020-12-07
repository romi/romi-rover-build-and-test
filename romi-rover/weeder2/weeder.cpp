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
#include <romi.h>
#include "Image.h"
#include "weeder.h"
#include "Weeder.h"
#include "ConfigurationFile.h"



messagelink_t *get_messagelink_cnc();

int weeder_init(int argc, char **argv)
{
        return 0;
}

void weeder_cleanup()
{
}

int weeder_onhoe(void *userdata,
                 messagelink_t *link,
                 json_object_t command,
                 membuf_t *message)
{
        ConfigurationFile configuration("config.json");
        Controller controller(get_messagelink_cnc());
        CNCProxy cnc(controller);
        CameraProxy camera("camera", "camera.jpg");


        Weeder weeder(camera, pipeline, cnc);
        std::string error_message;
        
        if (weeder.hoe(error_message)) {
                // OK
        } else {
                // Error
        }
        
        return 0;
}
