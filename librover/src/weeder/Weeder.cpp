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

#include "weeder/Weeder.h"

const double diameter_tool_default = 0.05;
// ToDo: Observation_id
const std::string observation_id = "row_1";


namespace romi {

        Weeder::Weeder(ICamera& camera,
                       IPipeline& pipeline,
                       ICNC& cnc,
                       double z0,
                       double speed,
                       ISession &session)
                : _camera(camera), _pipeline(pipeline), _cnc(cnc), _range(),
                  _z0(z0), _speed(speed), _diameter_tool(diameter_tool_default), session_(session)
        {
                _cnc.get_range(_range);
        }
        
        bool Weeder::hoe()
        {
                r_debug("Weeder::hoe");
                bool success = false;
                
                try {
                        session_.start(observation_id);
                        try_hoe();
                        success = true;
                        
                } catch (std::runtime_error& re) {

                        // Whatever happens, make sure the spindle
                        // stops and the arm goes back up
                        
                        r_debug("Weeder::hoe: catch: %s", re.what());
                        try {
                                stop_spindle_and_move_arm_up();
                        } catch (...) {}
                }
                
                return success;
        }
        
        void Weeder::try_hoe()
        {
                Image image;
                Path path;
                
                grab_image(image);
                compute_path(image, path);
                do_hoe(path);
        }

        void Weeder::grab_image(Image& image)
        {
                move_arm_to_camera_position();
                camera_grab(image);
        }

        void Weeder::camera_grab(Image& image)
        {
                if (!_camera.grab(image)) {
                        r_err("Weeder: grab failed");
                        throw std::runtime_error("Weeder: grab failed");
                }
        }

        void Weeder::compute_path(Image& image, Path& path)
        {
                Path normalized_path;
                analyse_image(image, normalized_path);
                adjust_path(normalized_path, path);
        }

        void Weeder::analyse_image(Image& image, Path& path)
        {
//                IFolder &folder = _filecabinet.start_new_folder();
                
                if (!_pipeline.run(session_, image, _diameter_tool, path)) {
                        r_err("Weeder: pipeline run failed");
                        throw std::runtime_error("Weeder: pipeline run failed");
                }
        }
        
        void Weeder::do_hoe(Path &path)
        {
                r_debug("Weeder::do_hoe");
                move_arm_to_start_position(path[0]);
                travel(path, 0.4);
                stop_spindle_and_move_arm_up();                
        }
        
        void Weeder::move_arm_to_camera_position()
        {
                stop_spindle_and_move_arm_up();
                moveto(0.0, _range.max.y(), 0.0);
        }

        void Weeder::move_arm_to_start_position(v3 p)
        {
                r_debug("Weeder::move_arm_to_start_position");

                stop_spindle_and_move_arm_up();
                moveto(p.x(), p.y(), 0.0);
                start_spindle();
                moveto(p.x(), p.y(), _z0);
        }

        void Weeder::stop_spindle_and_move_arm_up()
        {
                r_debug("Weeder::stop_spindle_and_move_arm_up");
                stop_spindle();
                moveto(ICNC::UNCHANGED, ICNC::UNCHANGED, 0.0);
        }
        
        void Weeder::adjust_path(Path &path, Path &out)
        {
                r_debug("Weeder::adjust_path");
                scale_to_range(path);
                rotate_path_to_starting_point(path, out);
                
                for (size_t i = 0; i < out.size(); i++) {
                        r_debug("Point: %.3f, %.3f, %.3f",
                                out[i].x(), out[i].y(), out[i].z());
                }
        }

        void Weeder::scale_to_range(Path &path)
        {
                r_debug("Weeder::scale_to_range");

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
        
        void Weeder::rotate_path_to_starting_point(Path &path, Path &out)
        {
                r_debug("Weeder::rotate_path_to_starting_point");
                v3 starting_point(0.0, _range.max.y(), _z0);
                int closest_index = path.closest_point(starting_point);
                if (closest_index >= 0)
                        path.rotate(out, (size_t)closest_index);
                else
                        throw std::runtime_error("rotate_path_to_starting_point empty path, no closest point");
        }
        
        void Weeder::travel(Path& path, double v)
        {
                if (!_cnc.travel(path, v)) {
                        r_err("Weeder: travel failed");
                        throw std::runtime_error("Weeder: travel failed");
                }
        }

        void Weeder::moveto(double x, double y, double z)
        {
                if (!_cnc.moveto(x, y, z, _speed)) {
                        r_err("Weeder: moveto failed");
                        throw std::runtime_error("Weeder: moveto failed");
                }
        }

        void Weeder::start_spindle()
        {
                if (!_cnc.spindle(1.0)) {
                        r_err("Weeder: start spindle failed");
                        throw std::runtime_error("Weeder: start spindle failed");
                }
        }
        
        void Weeder::stop_spindle()
        {
                if (!_cnc.spindle(0.0)) {
                        r_err("Weeder: stop spindle failed");
                        throw std::runtime_error("Weeder: stop spindle failed");
                }
        }

        bool Weeder::stop()
        {
                return _cnc.spindle(0.0);
        }
        
        bool Weeder::pause_activity()
        {
                return _cnc.pause_activity();
        }
        
        bool Weeder::continue_activity()
        {
                return _cnc.continue_activity();
        }
        
        bool Weeder::reset_activity()
        {
                return _cnc.reset_activity();
        }

        bool Weeder::power_up()
        {
                return _cnc.power_up();
        }
        
        bool Weeder::power_down()
        {
                return _cnc.power_down();
        }
        
        bool Weeder::stand_by()
        {
                return _cnc.stand_by();
        }
        
        bool Weeder::wake_up()
        {
                return _cnc.wake_up();
        }
}
