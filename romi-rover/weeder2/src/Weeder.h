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

#ifndef __ROMI_WEEDER_H
#define __ROMI_WEEDER_H

#include <string>
#include <IRPCHandler.h>
#include "ICamera.h"
#include "ICNC.h"
#include "IPipeline.h"
#include "IFileCabinet.h"

namespace romi {

        class Weeder : public rcom::IRPCHandler
        {
        protected:
                ICamera *_camera;
                IPipeline *_pipeline;
                ICNC *_cnc;
                CNCRange _range;
                double _z0;
                IFileCabinet &_filecabinet;
                
                void scale_to_range(Path &path);
                void shift_to_first_point(Path &path, Path &out);
                void adjust_path(Path &path, Path &out);
                bool path_in_range(Path &path);
                bool move_arm_to_camera_position();
                bool move_arm_to_start_position(Waypoint p);
                bool stop_spindle_and_move_arm_up();
                bool do_hoe(Path &som_path);

        public:
                Weeder(IConfiguration *configuration,
                       ICamera *camera,
                       IPipeline *pipeline,
                       ICNC *cnc,
                       CNCRange &range,
                       IFileCabinet &filecabinet)
                        : _camera(camera), _pipeline(pipeline),
                        _cnc(cnc), _range(range), _filecabinet(filecabinet) {
                        _z0 = configuration->get("weeder").num("z0");
                }
                
                virtual ~Weeder() override = default;
                
                bool hoe();

                // RPC interface
                void execute(const char *method, JSON &params,
                             JSON &result, rcom::RPCError &error) override;
        };
}

#endif // __ROMI_WEEDER_H
