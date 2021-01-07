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

#include "DebugWeedingSession.h"
#include "RoverWeeder.h"

namespace romi {

        bool RoverWeeder::move_arm_to_camera_position()
        {
                bool success = false;
                if (stop_spindle_and_move_arm_up()) {
                        if (_cnc->moveto(0.0, _range.max.y(), 0.0, 0.8)) {
                                success = true;
                        } else {
                                r_warn("RoverWeeder::move_arm_to_camera_position: "
                                       "moveto failed");
                        }
                } else {
                        r_warn("RoverWeeder::move_arm_to_camera_position: "
                               "stop_spindle_and_move_arm_up failed");
                }        
                return success;
        }

        bool RoverWeeder::move_arm_to_start_position(v3 p)
        {
                bool success = false;
                r_debug("RoverWeeder::move_arm_to_start_position");
                success = (stop_spindle_and_move_arm_up()
                           && _cnc->moveto(p.x(), p.y(), 0.0, 0.8)
                           && _cnc->spindle(1.0)
                           && _cnc->moveto(p.x(), p.y(), _z0, 0.8));
                if (!success) {
                        r_warn("RoverWeeder::move_arm_to_start_position: "
                               "stop_spindle_and_move_arm_up, "
                               "moveto or spindle failed");
                } 
                return success;
        }

        bool RoverWeeder::stop_spindle_and_move_arm_up()
        {
                bool success = false;
                r_debug("RoverWeeder::stop_spindle_and_move_arm_up");
                if (_cnc->spindle(0.0)) {
                        if (_cnc->moveto(CNC::UNCHANGED,
                                         CNC::UNCHANGED,
                                         0.0, 0.8)) {
                                success = true;
                        } else {
                                r_warn("RoverWeeder::stop_spindle_and_move_arm_up: "
                                       "moveto failed");
                        }
                } else {
                        r_warn("RoverWeeder::stop_spindle_and_move_arm_up: spindle failed");
                }
                return success;
        }

        void RoverWeeder::scale_to_range(Path &path)
        {
                r_debug("RoverWeeder::scale_to_range");

                // The y-axis of the rover is inverted with respect to
                // the image coordinates.
                path.invert_y();

                // The path has x,y coordinates in the range [0,1].
                // Map these coordinates to the physical dimensions of
                // the workspace.
                path.scale(_range.dimensions());
                path.translate(_range.min);
                path.set_z(_z0);
                
                if (!path.clamp(_range, 0.01))
                        throw std::runtime_error("Computed path out of range");
        }
        
        void RoverWeeder::rotate_path_to_starting_point(Path &path, Path &out)
        {
                r_debug("RoverWeeder::rotate_path_to_starting_point");
                v3 starting_point(0.0, _range.max.y(), _z0);
                int closest_index = path.closest_point(starting_point);
                path.rotate(out, closest_index);
        }
        
        void RoverWeeder::adjust_path(Path &path, Path &out)
        {
                r_debug("RoverWeeder::adjust_path");
                scale_to_range(path);
                rotate_path_to_starting_point(path, out);
                
                for (size_t i = 0; i < out.size(); i++) {
                        r_debug("Point: %.3f, %.3f, %.3f", out[i].x(), out[i].y(), out[i].z());
                }
        }
        
        bool RoverWeeder::do_hoe(Path &som_path)
        {
                Path path;
                bool success = false;
                
                r_debug("RoverWeeder::do_hoe");
                
                adjust_path(som_path, path);
       
                try {
                        r_debug("RoverWeeder::do_hoe: move_arm_to_start_position");
                        success = move_arm_to_start_position(path[0]);
                        
                        if (success) {
                                r_debug("RoverWeeder::do_hoe: _cnc->travel"); 
                                success = _cnc->travel(path, 0.4);
                        }
                        
                        if (success) {
                                r_debug("RoverWeeder::do_hoe: stop_spindle_and_move_arm_up");
                                success = stop_spindle_and_move_arm_up();
                        }
                        
                } catch (std::exception& e) {

                        // Whatever happens, make sure the spindle
                        // stops and the arm goes back up
                        
                        r_debug("RoverWeeder::do_hoe: catch");
                        stop_spindle_and_move_arm_up();
                        throw e;
                }
                
                return success;
        }
        
        bool RoverWeeder::hoe()
        {
                IFolder &folder = _filecabinet.start_new_folder();
                Image camera_image;
                Path path;
                bool success = false;
                
                r_debug("RoverWeeder::hoe");
                
                success = move_arm_to_camera_position();

                if (success) {
                        r_debug("RoverWeeder::hoe: _camera->grab");
                        success = _camera->grab(camera_image);
                }

                if (success) {
                        r_debug("RoverWeeder::hoe: _pipeline->run");
                        success = _pipeline->run(folder, camera_image, 0.05, path);
                }
                
                if (path.size()) {
                        success = do_hoe(path);
                }
                
                return success;
        }

        void RoverWeeder::try_hoe(rcom::RPCError &error)
        {
                try {
                        hoe();
                        error.code = 0;
                                
                } catch (std::exception& e) {
                        r_debug("RoverWeeder::exception: catched exception: %s", e.what());
                        error.code = 0;
                        error.message = e.what();
                }
        }
        
        void RoverWeeder::execute(const char *method, JsonCpp& params,
                                  JsonCpp& result, rcom::RPCError &error)
        {
                r_debug("RoverWeeder::execute");
                
                if (rstreq(method, "hoe")) {
                        try_hoe(error);
                        
                } else {
                        error.code = rcom::RPCError::MethodNotFound;
                        error.message = "Unknown method";
                }
        }
}
