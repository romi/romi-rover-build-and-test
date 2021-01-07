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

#include <vector>
#include <stdexcept>
#include "plotter.h"
#include "print.h"
#include "is_valid.h"
#include "Oquam.h"

namespace romi {
        
        Oquam::Oquam(CNCController& controller,
                     CNCRange& range,
                     const double *vmax,
                     const double *amax,
                     const double *scale_meters_to_steps, 
                     double path_max_deviation,
                     double path_slice_duration)
                : _controller(controller),
                  _file_cabinet(0),
                  _range(range),
                  _vmax(vmax),
                  _amax(amax),
                  _path_max_deviation(path_max_deviation),
                  _path_slice_duration(path_slice_duration)
        {
                _path_max_slice_duration = 32.0;
                
                vcopy(_scale_meters_to_steps, scale_meters_to_steps);
                
                _script_count = 0;

                if (!homing()) {
                        r_err("Oquam:: Homing failed!");
                        throw std::runtime_error("Homing failed");
                }
        }
                
        bool Oquam::get_range(CNCRange &range)
        {
                range = _range;
                return true;
        }

        bool Oquam::get_position(int32_t *position) 
        {
                return _controller.get_position(position);
        }
        
        bool Oquam::get_position(v3& position) 
        {
                int32_t p[3];
                bool success = get_position(p);
                if (success) {
                        position.set(p[0] / _scale_meters_to_steps[0],
                                     p[1] / _scale_meters_to_steps[1],
                                     p[2] / _scale_meters_to_steps[2]);
                }
                return success;
        }

        v3 Oquam::assert_get_position() 
        {
                v3 position;
                if (!get_position(position)) {
                        r_err("Oquam:: get_position failed!");
                        throw std::runtime_error("get_position failed");
                }
                return position;
        }
        
        bool Oquam::moveto(double x, double y, double z, double relative_speed)
        {
                SynchonizedCodeBlock synchronize(_m);
                return moveto_synchronized(x, y, z, relative_speed);
        }

        bool Oquam::moveto_synchronized(double x, double y, double z, double rel_speed)
        {
                bool success = false;
                try {
                        success = do_moveto(x, y, z, rel_speed);
                } catch (std::runtime_error& re) {
                        r_err("Oquam::moveto_synchronized: %s", re.what());
                }
                return success;
        }

        bool Oquam::do_moveto(double x, double y, double z, double rel_speed)
        {
                Path path;
                v3 p = moveto_determine_xyz(x, y, z);
                path.push_back(p);
                return travel_synchronized(path, rel_speed);
        }
        
        v3 Oquam::moveto_determine_xyz(double x, double y, double z)
        {
                v3 p = assert_get_position();
                if (x != UNCHANGED)
                        p.x() = x;
                if (y != UNCHANGED)
                        p.y() = y;
                if (z != UNCHANGED)
                        p.z() = z;
                return p;
        }
        
        bool Oquam::is_zero(int16_t *params)
        {
                return (params[0] == 0)
                        || ((params[1] == 0)
                            && (params[2] == 0)
                            && (params[3] == 0.0));
        }
        
        bool Oquam::spindle(double speed)
        {
                // TODO
                r_err("Oquam::spindle NOT IMPLEMENTED YET!");
                return true;
        }

        bool Oquam::homing()
        {
                SynchonizedCodeBlock synchronize(_m);
                return _controller.homing();
        }
                
        bool Oquam::travel(Path &path, double relative_speed)
        {
                SynchonizedCodeBlock synchronize(_m);
                return travel_synchronized(path, relative_speed);
        }
        
        bool Oquam::travel_synchronized(Path &path, double relative_speed) 
        {
                bool success = false;
                
                try {
                        do_travel(path, relative_speed); 
                        success = true;
                        
                } catch (std::runtime_error& e) {
                        r_debug("Oquam::travel_synchronized: error: %s", e.what());
                }
                
                return success;
        }

        void Oquam::assert_relative_speed(double relative_speed) 
        {
                if (relative_speed <= 0.0 || relative_speed > 1.0)
                        throw std::runtime_error("Oquam: invalid speed");
        }
        
        void Oquam::assert_in_range(v3 p) 
        {
                if (!_range.is_inside(p)) {
                        r_warn("Oquam: Point[%d]: out of bounds: "
                               "(%0.4f, %0.4f, %0.4f)", p.x(), p.y(), p.z());
                        throw std::runtime_error("Point out of bounds");
                }
        }
        
        void Oquam::do_travel(Path &path, double relative_speed) 
        {
                assert_relative_speed(relative_speed); 
                
                v3 start_position = assert_get_position();
                Script script(start_position);
                
                v3 vmax;
                vmax = _vmax * relative_speed;
                
                double speed_ms = norm(_vmax * relative_speed);
                
                convert_path_to_script(path, speed_ms, script);
                convert_script(script, vmax);
                store_script(script);
                check_script(script, vmax); 
                execute_script(script);
                wait_end_of_script(script); 
        }

        void Oquam::convert_path_to_script(Path &path, double speed, Script& script) 
        {
                for (size_t i = 0; i < path.size(); i++) {
                        assert_in_range(path[i]); 
                        script.moveto(path[i], speed);
                }
        }

        void Oquam::convert_script(Script& script, v3& vmax) 
        {
                script.convert(vmax.values(), _amax.values(), _path_max_deviation,
                               _path_slice_duration, _path_max_slice_duration); 
        }

        void Oquam::store_script(Script& script) 
        {
                if (_file_cabinet) {
                        IFolder &folder = _file_cabinet->start_new_folder();
                        store_script_svg(folder, script); 
                        store_script_json(folder, script); 
                }
        }

        void Oquam::store_script_svg(IFolder &folder, Script& script) 
        {
                membuf_t *svg = plot_to_mem(script, _range, _vmax.values(), _amax.values());
                if (svg != 0) {
                        folder.store_svg("path", membuf_data(svg), membuf_len(svg));
                        delete_membuf(svg);
                } else {
                        r_warn("Oquam::store_script: plot failed");
                }
        }

        void Oquam::store_script_json(IFolder &folder, Script& script) 
        {
                membuf_t *text = new_membuf();
                print(script, text);
                folder.store_txt("script", membuf_data(text), membuf_len(text));
                delete_membuf(text);
        }

        void Oquam::check_script(Script& script, v3& vmax) 
        {
                if (!is_valid(script, _path_max_slice_duration, _range,
                              vmax.values(), _amax.values())) {
                        r_err("Oquam::convert_script: generated script is invalid");
                        throw std::runtime_error("is_valid(script) failed");
                }
        }
        
        void Oquam::execute_script(Script& script) 
        {
                if (script.count_slices() > 0) {
                        Section& section = script.get_slice(0);
                
                        // Initialize the start position
                        int32_t pos_steps[3];
                        convert_position_to_steps(section.p0, pos_steps); 

                        for (size_t k = 0; k < script.count_slices(); k++) {
                                execute_move(script.get_slice(k), pos_steps);
                        }
                }
        }

        void Oquam::execute_move(Section& section, int32_t *pos_steps)
        {
                int32_t p1[3];
                convert_position_to_steps(section.p1, p1); 

                int16_t params[4];
                params[0] = (int16_t) (1000.0 * section.duration);
                params[1] = (int16_t) (p1[0] - pos_steps[0]);
                params[2] = (int16_t) (p1[1] - pos_steps[1]);
                params[3] = (int16_t) (p1[2] - pos_steps[2]);
                
                if (!is_zero(params)) {
                        
                        assert_move(params);
                        
                        // Update the current position
                        pos_steps[0] = p1[0];
                        pos_steps[1] = p1[1];
                        pos_steps[2] = p1[2];
                }
        }
        
        void Oquam::assert_move(int16_t *params)
        {
                if (!_controller.move(params[0], params[1], params[2], params[3])) {
                        r_err("Oquam: move failed");
                        throw std::runtime_error("Oquam: move failed");
                }
        }

        void Oquam::convert_position_to_steps(double *position, int32_t *steps) 
        {
                double *scale = _scale_meters_to_steps;
                steps[0] = (int32_t) (position[0] * scale[0]);
                steps[1] = (int32_t) (position[1] * scale[1]);
                steps[2] = (int32_t) (position[2] * scale[2]);
        }

        void Oquam::wait_end_of_script(Script& script) 
        {
                double duration = script.get_duration();
                double timeout = 60.0 + 1.5 * duration;
                assert_synchronize(timeout);
        }

        void Oquam::assert_synchronize(double timeout)
        {
                if (!_controller.synchronize(timeout)) {
                        r_err("Oquam: synchronize failed");
                        throw std::runtime_error("Oquam: synchronize failed");
                }
        }

        bool Oquam::stop_execution()
        {
                return _controller.stop_execution();
        }
        
        bool Oquam::continue_execution()
        {
                return _controller.continue_execution();
        }
        
        bool Oquam::reset()
        {
                return _controller.reset();
        }
}
