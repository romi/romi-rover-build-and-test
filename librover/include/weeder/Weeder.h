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

#ifndef __ROMI_DEFAULT_WEEDER_H
#define __ROMI_DEFAULT_WEEDER_H

#include <string>

#include "api/ICamera.h"
#include "api/ICNC.h"
#include "api/IWeeder.h"
#include "IFileCabinet.h"
#include "weeder/IPipeline.h"
#include "session/ISession.h"

namespace romi {

        class Weeder : public IWeeder
        {
        protected:
                ICamera& _camera;
                IPipeline& _pipeline;
                ICNC& _cnc;
                CNCRange _range;
                double _z0;
                double _speed;
                double _diameter_tool;
                ISession &session_;

                void scale_to_range(Path &path);
                void rotate_path_to_starting_point(Path &path, Path &out);
                void adjust_path(Path &path, Path &out);
                void move_arm_to_camera_position();
                void move_arm_to_start_position(v3 p);
                void stop_spindle_and_move_arm_up();
                void do_hoe(Path &path);
                void try_hoe();
                void moveto(double x, double y, double z);
                void start_spindle();
                void stop_spindle();
                void travel(Path& path, double v);
                void grab_image(Image& image);
                void camera_grab(Image& image);
                void analyse_image(Image& image, Path& path);
                void compute_path(Image& image, Path& path);
                void store_svg(Path& path);
                void store_svg_path(rpp::MemBuffer& buffer, Path& path);

        public:

                Weeder(ICamera& camera, IPipeline& pipeline, ICNC& cnc,
                   double z0, double speed, ISession &session);
                
                ~Weeder() override = default;

                // IWeeder interface
                bool hoe() override;
                bool stop() override;
                
                // IActivity interface
                bool pause_activity() override;
                bool continue_activity() override;
                bool reset_activity() override;

                // Power device interface
                bool power_up() override;
                bool power_down() override;
                bool stand_by() override;
                bool wake_up() override;
        };
}

#endif // __ROMI_DEFAULT_WEEDER_H
