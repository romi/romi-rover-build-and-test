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
#include "weeder/DefaultWeeder.h"

namespace romi {

        DefaultWeeder::DefaultWeeder(Camera& camera,
                                 IPipeline& pipeline,
                                 CNC& cnc,
                                 double z0,
                                 double speed,
                                 IFileCabinet &filecabinet)
                : _camera(camera), _pipeline(pipeline), _cnc(cnc),
                  _z0(z0), _speed(speed), _filecabinet(filecabinet)
        {
                _diameter_tool = 0.05;
                _cnc.get_range(_range);
        }
        
        bool DefaultWeeder::hoe()
        {
                r_debug("DefaultWeeder::hoe");
                bool success = false;
                
                try {

                        try_hoe();
                        success = true;
                        
                } catch (std::runtime_error& re) {

                        // Whatever happens, make sure the spindle
                        // stops and the arm goes back up
                        
                        r_debug("DefaultWeeder::hoe: catch: %s", re.what());
                        try {
                                stop_spindle_and_move_arm_up();
                        } catch (...) {}
                }
                
                return success;
        }
        
        void DefaultWeeder::try_hoe()
        {
                Image image;
                Path path;
                
                grab_image(image);
                compute_path(image, path);
                do_hoe(path);
        }

        void DefaultWeeder::grab_image(Image& image)
        {
                move_arm_to_camera_position();
                camera_grab(image);
        }

        void DefaultWeeder::camera_grab(Image& image)
        {
                if (!_camera.grab(image)) {
                        r_err("DefaultWeeder: grab failed");
                        throw std::runtime_error("DefaultWeeder: grab failed");
                }
        }

        void DefaultWeeder::compute_path(Image& image, Path& path)
        {
                Path normalized_path;
                analyse_image(image, normalized_path);
                adjust_path(normalized_path, path);
        }

        void DefaultWeeder::analyse_image(Image& image, Path& path)
        {
                IFolder &folder = _filecabinet.start_new_folder();
                
                if (!_pipeline.run(folder, image, _diameter_tool, path)) {
                        r_err("DefaultWeeder: pipeline run failed");
                        throw std::runtime_error("DefaultWeeder: pipeline run failed");
                }
        }
        
        void DefaultWeeder::do_hoe(Path &path)
        {
                r_debug("DefaultWeeder::do_hoe");
                move_arm_to_start_position(path[0]);
                travel(path, 0.4);
                stop_spindle_and_move_arm_up();                
        }
        
        void DefaultWeeder::move_arm_to_camera_position()
        {
                stop_spindle_and_move_arm_up();
                moveto(0.0, _range.max.y(), 0.0);
        }

        void DefaultWeeder::move_arm_to_start_position(v3 p)
        {
                r_debug("DefaultWeeder::move_arm_to_start_position");

                stop_spindle_and_move_arm_up();
                moveto(p.x(), p.y(), 0.0);
                start_spindle();
                moveto(p.x(), p.y(), _z0);
        }

        void DefaultWeeder::stop_spindle_and_move_arm_up()
        {
                r_debug("DefaultWeeder::stop_spindle_and_move_arm_up");
                stop_spindle();
                moveto(CNC::UNCHANGED, CNC::UNCHANGED, 0.0);
        }
        
        void DefaultWeeder::adjust_path(Path &path, Path &out)
        {
                r_debug("DefaultWeeder::adjust_path");
                scale_to_range(path);
                rotate_path_to_starting_point(path, out);
                
                for (size_t i = 0; i < out.size(); i++) {
                        r_debug("Point: %.3f, %.3f, %.3f",
                                out[i].x(), out[i].y(), out[i].z());
                }
        }

        void DefaultWeeder::scale_to_range(Path &path)
        {
                r_debug("DefaultWeeder::scale_to_range");

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
        
        void DefaultWeeder::rotate_path_to_starting_point(Path &path, Path &out)
        {
                r_debug("DefaultWeeder::rotate_path_to_starting_point");
                v3 starting_point(0.0, _range.max.y(), _z0);
                int closest_index = path.closest_point(starting_point);
                path.rotate(out, closest_index);
        }
        
        void DefaultWeeder::travel(Path& path, double v)
        {
                if (!_cnc.travel(path, v)) {
                        r_err("DefaultWeeder: travel failed");
                        throw std::runtime_error("DefaultWeeder: travel failed");
                }
        }

        void DefaultWeeder::moveto(double x, double y, double z)
        {
                if (!_cnc.moveto(x, y, z, _speed)) {
                        r_err("DefaultWeeder: moveto failed");
                        throw std::runtime_error("DefaultWeeder: moveto failed");
                }
        }

        void DefaultWeeder::start_spindle()
        {
                if (!_cnc.spindle(1.0)) {
                        r_err("DefaultWeeder: start spindle failed");
                        throw std::runtime_error("DefaultWeeder: start spindle failed");
                }
        }
        
        void DefaultWeeder::stop_spindle()
        {
                if (!_cnc.spindle(0.0)) {
                        r_err("DefaultWeeder: stop spindle failed");
                        throw std::runtime_error("DefaultWeeder: stop spindle failed");
                }
        }

        bool DefaultWeeder::stop()
        {
                return _cnc.spindle(0.0);
        }
        
        bool DefaultWeeder::pause_activity()
        {
                return _cnc.pause_activity();
        }
        
        bool DefaultWeeder::continue_activity()
        {
                return _cnc.continue_activity();
        }
        
        bool DefaultWeeder::reset_activity()
        {
                return _cnc.reset_activity();
        }

        bool DefaultWeeder::power_up()
        {
                return _cnc.power_up();
        }
        
        bool DefaultWeeder::power_down()
        {
                return _cnc.power_down();
        }
        
        bool DefaultWeeder::stand_by()
        {
                return _cnc.stand_by();
        }
        
        bool DefaultWeeder::wake_up()
        {
                return _cnc.wake_up();
        }
}
