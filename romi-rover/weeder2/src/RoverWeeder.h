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

#ifndef __ROMI_ROVER_WEEDER_H
#define __ROMI_ROVER_WEEDER_H

#include <string>
#include <IRPCHandler.h>
#include "Camera.h"
#include "CNC.h"
#include "IPipeline.h"
#include "IFileCabinet.h"
#include "Weeder.h"

namespace romi {

        class RoverWeeder : public rcom::IRPCHandler
        {
        protected:
                Camera *_camera;
                IPipeline *_pipeline;
                CNC *_cnc;
                CNCRange _range;
                double _z0;
                IFileCabinet &_filecabinet;
                
                void scale_to_range(Path &path);
                void rotate_path_to_starting_point(Path &path, Path &out);
                void adjust_path(Path &path, Path &out);
                bool move_arm_to_camera_position();
                bool move_arm_to_start_position(v3 p);
                bool stop_spindle_and_move_arm_up();
                bool do_hoe(Path &som_path);
                void try_hoe(rcom::RPCError &error);

        public:
                RoverWeeder(Camera *camera,
                            IPipeline *pipeline,
                            CNC *cnc,
                            CNCRange &range,
                            double z0,
                            IFileCabinet &filecabinet)
                        : _camera(camera), _pipeline(pipeline), _cnc(cnc),
                          _range(range), _z0(z0), _filecabinet(filecabinet) {
                }
                
                virtual ~RoverWeeder() override = default;
                
                bool hoe();

                // RPC interface
                void execute(const char *method, JsonCpp& params,
                             JsonCpp& result, rcom::RPCError &error) override;
        };
}

#endif // __ROMI_ROVER_WEEDER_H
