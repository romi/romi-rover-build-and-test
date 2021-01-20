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

#ifndef __ROMI_ROVER_OPTIONS_H
#define __ROMI_ROVER_OPTIONS_H

#include "GetOpt.h"

namespace romi {

        class RoverOptions : public GetOpt
        {
        public:
                static constexpr const char* config = "config";
                static constexpr const char* script = "script";
                static constexpr const char* soundfont = "soundfont";
                static constexpr const char* camera_image = "camera-image";
                static constexpr const char* session_directory = "session-directory";
                static constexpr const char* joystick_device = "joystick-device";
                static constexpr const char* camera_device = "camera-device";
                static constexpr const char* display_device = "display-device";
                static constexpr const char* cnc_device = "cnc-device";
                static constexpr const char* navigation_device = "navigation-device";


                RoverOptions();
                ~RoverOptions() override = default;
                
                void exit_if_help_requested();
                
                //const char *get_config_file(Options& options);

        protected:
        };
}

#endif // __ROMI_ROVER_OPTIONS_H

