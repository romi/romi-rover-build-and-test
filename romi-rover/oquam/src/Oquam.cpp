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
                     double path_slice_interval)
                : _controller(controller),
                  _file_cabinet(0),
                  _range(range),
                  _path_max_deviation(path_max_deviation),
                  _path_slice_interval(path_slice_interval)
        {
                vcopy(_vmax, vmax);
                vcopy(_amax, amax);
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
        
        bool Oquam::get_position(double *position) 
        {
                int32_t p[3];
                bool success = get_position(p);
                if (success) {
                        for (int i = 0; i < 3; i++)
                                position[i] = (double) p[i] / _scale_meters_to_steps[i];
                }
                return success;
        }

        void Oquam::assert_get_position(double *position) 
        {
                if (!get_position(position)) {
                        r_err("Oquam:: get_position failed!");
                        throw std::runtime_error("get_position failed");
                }
        }
        
        bool Oquam::moveto(double x, double y, double z, double relative_speed)
        {
                SynchonizedCodeBlock synchronize(_m);
                return moveto_synchronized(x, y, z, relative_speed);
        }

        bool Oquam::moveto_synchronized(double x, double y, double z, double rel_speed)
        {
                r_debug("Oquam::moveto_synchronized");
                bool success = false;
                
                try {
                        do_moveto(x, y, z, rel_speed);
                        success = true;
                        
                } catch (std::runtime_error& e) {
                        r_err("Oquam::moveto failed: %s", e.what());
                }
                return success;
        }
        
        void Oquam::do_moveto(double x, double y, double z, double rel_speed)
        {
                r_debug("Oquam::do_moveto");
                int16_t params[4];
                
                assert_relative_speed(rel_speed);
                assert_in_range(x, y, z);
                
                compute_steps_and_millis(x, y, z, rel_speed, params);
                if (!is_zero(params)) {
                        move_and_synchronize(params);
                } 
        }

        void Oquam::move_and_synchronize(int16_t *params)
        {
                assert_move(params);
                assert_synchronize(2.0 * get_duration(params));
        }

        void Oquam::assert_move(int16_t *params)
        {
                if (!_controller.move(params[0], params[1], params[2], params[3])) {
                        r_err("Oquam: move failed");
                        throw std::runtime_error("Oquam: move failed");
                }
        }

        void Oquam::assert_synchronize(double timeout)
        {
                if (!_controller.synchronize(timeout)) {
                        r_err("Oquam: synchronize failed");
                        throw std::runtime_error("Oquam: synchronize failed");
                }
        }

        bool Oquam::is_zero(int16_t *params)
        {
                return (params[1] == 0) && (params[2] == 0) && (params[3] == 0.0);
        }

        double Oquam::get_duration(int16_t *params)
        {
                return params[0] / 1000.0;
        }

        void Oquam::compute_steps_and_millis(double x, double y, double z,
                                             double rel_speed,
                                             int16_t *results)
        {
                double position[3];
                assert_get_position(position);
                
                double dx[3] = {0.0, 0.0, 0.0};
                if (x != UNCHANGED)
                        dx[0] = x - position[0];
                        
                if (y != UNCHANGED)
                        dx[1] = y - position[1];
                        
                if (z != UNCHANGED)
                        dx[2] = z - position[2];
                        
                double v[3];
                smul(v, _vmax, rel_speed);
                
                double dt[3];
                vabs(dt, dx);
                vdiv(dt, dt, v);
                                
                double duration = ::vmax(dt);
                double *scale = _scale_meters_to_steps;
                                
                results[0] = (int16_t) (1000.0 * duration);
                results[1] = (int16_t) (dx[0] * scale[0]);
                results[2] = (int16_t) (dx[1] * scale[1]);
                results[3] = (int16_t) (dx[2] * scale[2]);
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
        
        void Oquam::assert_in_range(double x, double y, double z) 
        {
                if (!_range.is_valid(x, y, z)) {
                        r_warn("Oquam: Point[%d]: out of bounds: "
                               "(%0.4f, %0.4f, %0.4f)", x, y, z);
                        throw std::runtime_error("Point out of bounds");
                }
        }
        
        void Oquam::do_travel(Path &path, double relative_speed) 
        {
                r_debug("Oquam::travel_synchronized");
                
                assert_relative_speed(relative_speed); 
                
                double start_position[3];
                assert_get_position(start_position);
                        
                Script script(start_position);
                
                double vmax[3];
                smul(vmax, _vmax, relative_speed);
                
                double speed_ms = norm(vmax);
                
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
                        double x = path[i].x;
                        double y = path[i].y;
                        double z = path[i].z;

                        assert_in_range(x, y, z); 
                        script.moveto(x, y, z, speed);
                }
        }

        void Oquam::convert_script(Script& script, double *vmax) 
        {
                r_debug("Oquam::convert_script: script_convert");
                script.convert(vmax, _amax, _path_max_deviation,
                               _path_slice_interval, 32.0);                
        }

        void Oquam::store_script(Script& script) 
        {
                if (_file_cabinet) {

                        char name[64];
                        rprintf(name, 64, "oquam-%04d", _script_count++);
                        
                        IFolder &folder = _file_cabinet->start_new_folder();
                        membuf_t *svg = plot_to_mem(&script, _range, _vmax, _amax);
                        if (svg != 0) {
                                folder.store_svg("path", membuf_data(svg),
                                                 membuf_len(svg));
                                delete_membuf(svg);
                        } else {
                                r_warn("Oquam::store_script: plot failed");
                        }

                        membuf_t *text = new_membuf();
                        print(script, text);
                        folder.store_txt("script", membuf_data(text), membuf_len(text));
                        delete_membuf(text);
                }
        }

        void Oquam::check_script(Script& script, double *vmax) 
        {
                if (!is_valid(script, 32.0, _range, vmax, _amax)) {
                        r_err("Oquam::convert_script: generated script is invalid");
                        throw std::runtime_error("is_valid(script) failed");
                }
        }
        
        void Oquam::execute_script(Script& script) 
        {
                int32_t pos_steps[3];
                Section& section = script.slices[0];
                double *scale = _scale_meters_to_steps;
                
                // Initialize the start position
                pos_steps[0] = (int32_t) (section.p0[0] * scale[0]);
                pos_steps[1] = (int32_t) (section.p0[1] * scale[1]);
                pos_steps[2] = (int32_t) (section.p0[2] * scale[2]);

                // r_debug("Start: Abs(%.3f,%.3f,%.3f)=(%d,%d,%d)",
                //         section.p0[0], section.p0[1], section.p0[2],
                //         pos_steps[0], pos_steps[1], pos_steps[2]);

                for (size_t k = 0; k < script.slices.size(); k++) {
                        // r_debug("Section %d:", k);
                        execute_move(script.slices[k], pos_steps);
                }

                // r_debug("End: Abs(%d,%d,%d)",
                //         pos_steps[0], pos_steps[1], pos_steps[2]);
        }

        void Oquam::execute_move(Section& section, int32_t *pos_steps)
        {
                int32_t p1[3];
                p1[0] = (int32_t) (section.p1[0] * _scale_meters_to_steps[0]);
                p1[1] = (int32_t) (section.p1[1] * _scale_meters_to_steps[1]);
                p1[2] = (int32_t) (section.p1[2] * _scale_meters_to_steps[2]);

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

        void Oquam::wait_end_of_script(Script& script) 
        {
                double duration = script_duration(script);
                double timeout = 60.0 + 1.5 * duration;
                assert_synchronize(timeout);
        }

        double Oquam::script_duration(Script& script)
        {
                Section& section = script.slices.back();
                return section.end_time();
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
