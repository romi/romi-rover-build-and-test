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
#include "Weeder.h"

namespace romi {

        void Weeder::move_arm_to_camera_position()
        {
                r_debug("Weeder::move_arm_to_camera_position");
                move_arm_up();
                _cnc->moveto(0.0, _range._y[1], 0.0, 0.8);
        }

        void Weeder::move_arm_to_start_position(Waypoint p)
        {
                r_debug("Weeder::move_arm_to_start_position");
                move_arm_up();
                _cnc->moveto(p.x, p.y, 0.0, 0.8);
                _cnc->spindle(1.0);
                _cnc->moveto(p.x, p.y, _z0, 0.8);
        }

        void Weeder::move_arm_up()
        {
                r_debug("Weeder::move_arm_up");
                _cnc->spindle(0.0);
                _cnc->moveto(ICNC::UNCHANGED, ICNC::UNCHANGED, 0.0, 0.8);
        }
        
        bool Weeder::path_in_range(Path &path)
        {
                r_debug("Weeder::path_in_range");
                bool valid = true;
                for (size_t i = 0; i < path.size(); i++) {
                        double x = path[i].x;
                        double y = path[i].y;
                        double z = path[i].z;
                        
                        if (!_range.is_valid(x, y, z)) {
                                
                                r_err("Point: %.3f, %.3f, %.3f: "
                                      "*** Out of range ***: (%.3f, %.3f), "
                                      "(%.3f, %.3f), (%.3f, %.3f)",
                                      x, y, z,
                                      _range._x[0], _range._x[1],
                                      _range._y[0], _range._y[1],
                                      _range._z[0], _range._z[1]);
                        
                                if (_range.error(x, y, z) < 0.01) {
                                        // Small error: clip
                                        // TODO: call cnc_range to do this
                                        if (x < _range._x[0])
                                                x = _range._x[0];
                                        if (x > _range._x[1])
                                                x = _range._x[1];
                                        if (y < _range._y[0])
                                                y = _range._y[0];
                                        if (y > _range._y[1])
                                                y = _range._y[1];
                                        if (z < _range._z[0])
                                                z = _range._z[0];
                                        if (z > _range._z[1])
                                                z = _range._z[1];

                                        path[i].x = x;
                                        path[i].y = y;
                                        path[i].z = z;
                                        
                                } else {
                                        r_err("Computed point out of range: "
                                              "%.3f, %.3f, %.3f", x, y, z);
                                        valid = false;
                                        break;
                                } 
                        }
                }
                return valid;
        }

        void Weeder::scale_to_range(Path &path)
        {
                r_debug("Weeder::scale_to_range");

                // The y-axis of the rover is inverted with respect to
                // the image coordinates.
                path_invert_y(path);

                // The path has x,y coordinates in the range [0,1].
                // Map these coordinates to the physical dimensions of
                // the workspace.
                path_scale(path, _range._x[1] - _range._x[0],
                           _range._y[1] - _range._y[0], 1.0);
                path_translate(path, _range._x[0], _range._y[0], 0.0);
                
                path_set_z(path, _z0);
                
                if (!path_in_range(path))
                        throw std::runtime_error("Computed path out of range");
        }
        
        void Weeder::shift_to_first_point(Path &path, Path &out)
        {
                r_debug("Weeder::shift_to_first_point");
                size_t start_point = path_closest_point(path, 0.0, _range._y[1], _z0);
                path_shift(path, out, start_point);
        }
        
        void Weeder::adjust_path(Path &path, Path &out)
        {
                r_debug("Weeder::adjust_path");
                scale_to_range(path);
                shift_to_first_point(path, out);
                
                for (size_t i = 0; i < out.size(); i++) {
                        r_debug("Point: %.3f, %.3f, %.3f", out[i].x, out[i].y, out[i].z);
                }
        }
        
        void Weeder::do_hoe(Path &som_path)
        {
                Path path;
                
                r_debug("Weeder::do_hoe");
                adjust_path(som_path, path);
       
                try {
                        r_debug("Weeder::do_hoe: move_arm_to_start_position");
                        move_arm_to_start_position(path[0]);
                        r_debug("Weeder::do_hoe: _cnc->travel");
                        _cnc->travel(path, 0.4);
                        r_debug("Weeder::do_hoe: move_arm_up");
                        move_arm_up();
                                
                } catch (std::exception& e) {
                        r_debug("Weeder::do_hoe: catch");
                        move_arm_up();
                        throw e;
                }
        }
        
        void Weeder::hoe()
        {
                IFolder &folder = _filecabinet.start_new_folder();
                Image camera_image;
                Path path;
                
                r_debug("Weeder::hoe");
                
                move_arm_to_camera_position();
                
                r_debug("Weeder::hoe: _camera->grab");
                _camera->grab(camera_image);

                r_debug("Weeder::hoe: _pipeline->run");
                _pipeline->run(folder, camera_image, 0.05, path);

                if (path.size()) {
                        do_hoe(path);
                }
        }
        
        void Weeder::execute(JSON &cmd, JSON &result)
        {
                r_debug("Weeder::execute");
                
                const char *command = cmd.str("command");
                if (rstreq(command, "hoe")) {
                        hoe();
                        result = JSON::parse("{\"status\":\"ok\"}");
                } else {
                        result = JSON::parse("{\"status\":\"error\", "
                                             "\"message\": \"Unknown command\"}");
                }
        }
}
