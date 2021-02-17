/*
  romi-rover

  Copyright (C) 2019-2020 Sony Computer Science Laboratories
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

#ifndef __OQUAM_FACTORY_H
#define __OQUAM_FACTORY_H

#include <memory>
#include <RomiSerialClient.h>
#include <RSerial.h>

#include "Options.h"
#include "oquam/CNCController.h"

namespace romi {

        class OquamFactory
        {
        protected:
                std::shared_ptr<RSerial> _serial;
                std::unique_ptr<RomiSerialClient> _romi_serial;
                std::unique_ptr<CNCController> _controller;
                
                void instantiate_controller(Options &options, JsonCpp &config);
                void instantiate_controller(const char *controller_classname,
                                            Options &options, JsonCpp &config);
                const char *get_controller_classname_in_config(JsonCpp &config);
                void instantiate_fake_controller();
                void instantiate_stepper_controller(Options &options, JsonCpp &config);
                const char *get_stepper_controller_device(Options &options,
                                                          JsonCpp &config);
                const char *get_stepper_controller_device_in_config(JsonCpp &config);

        public:

                OquamFactory();
                virtual ~OquamFactory();
                
                CNCController& create_controller(Options& options, JsonCpp& config);

        };
}

#endif // __OQUAM_FACTORY_H
