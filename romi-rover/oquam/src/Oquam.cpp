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
#include "Oquam.h"

namespace romi {
        
        Oquam::Oquam(CNCController *controller,
                     const double *xmin, const double *xmax,
                     const double *vmax, const double *amax,
                     const double *scale_meters_to_steps, 
                     double path_max_deviation,
                     double path_slice_interval)
                : _controller(controller),
                  _file_cabinet(0),
                  _path_max_deviation(path_max_deviation),
                  _path_slice_interval(path_slice_interval)
        {
                if (_controller == 0)
                        throw std::runtime_error("Oquam: invalid CNC controller");
                
                vcopy(_xmin, xmin);
                vcopy(_xmax, xmax);
                vcopy(_vmax, vmax);
                vcopy(_amax, amax);
                vcopy(_scale_meters_to_steps, scale_meters_to_steps);
                _script_count = 0;
                
                homing();
        }

        bool Oquam::moveto_synchronized(double x, double y, double z, double rel_speed)
        {
                r_debug("Oquam::moveto_synchronized");
                bool success = false;
                
                if (rel_speed > 0.0 && rel_speed <= 1.0) {
                        
                        double position[3];
                        get_position(position);
                
                        double dx[3] = {0.0, 0.0, 0.0};
                        if (x != UNCHANGED)
                                dx[0] = x - position[0];
                        
                        if (y != UNCHANGED)
                                dx[1] = y - position[1];
                        
                        if (z != UNCHANGED)
                                dx[2] = z - position[2];
                        
                        if ((dx[0] != 0.0) || (dx[1] != 0.0) || (dx[2] != 0.0)) {
                                
                                double v[3];
                                smul(v, _vmax, rel_speed);
                
                                double dt[3];
                                vabs(dt, dx);
                                vdiv(dt, dt, v);
                                
                                double duration = ::vmax(dt);
                                double *scale = _scale_meters_to_steps;
                                
                                int16_t millis = (int16_t) (1000.0 * duration);
                                int16_t step_x = (int16_t) (dx[0] * scale[0]);
                                int16_t step_y = (int16_t) (dx[1] * scale[1]);
                                int16_t step_z = (int16_t) (dx[2] * scale[2]);
                                
                                if (_controller->move(millis, step_x, step_y, step_z)) {
                                        if (_controller->synchronize(2.0 * duration)) {
                                                success = true;
                                        } else {
                                                r_err("Oquam::moveto_synchronized: synchronize failed");
                                        }
                                } else {
                                        r_err("Oquam::moveto_synchronized: moveto failed");
                                }
                        }
                } else {
                        r_err("Oquam::moveto_synchronized: invalid speed: %f", rel_speed);
                }
                return success;
        }

        void Oquam::store_script(Script& script) 
        {
                if (_file_cabinet) {

                        char name[64];
                        rprintf(name, 64, "oquam-%04d", _script_count++);
                        
                        IFolder &folder = _file_cabinet->start_new_folder();
                        membuf_t *svg = plot_to_mem(&script, _xmin, _xmax, _vmax,
                                                    _amax, _scale_meters_to_steps);
                        if (svg != 0) {
                                folder.store_svg("path", membuf_data(svg),
                                                 membuf_len(svg));
                                delete_membuf(svg);
                        } else {
                                r_warn("Oquam::store_script: plot failed");
                        }

                        membuf_t *text = new_membuf();
                        script.print_segments(text);
                        folder.store_txt("segments", membuf_data(text), membuf_len(text));

                        membuf_clear(text);
                        script.print_atdc(text);
                        folder.store_txt("atdc", membuf_data(text), membuf_len(text));

                        membuf_clear(text);
                        script.print_slices(text);
                        folder.store_txt("slices", membuf_data(text), membuf_len(text));
                        
                        delete_membuf(text);
                }
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

        double Oquam::get_absolute_speed(double relative_speed) 
        {
                double v[3];
                smul(v, _vmax, relative_speed);
                double speed_ms = ::vmax(v);
                return speed_ms;
        }

        void Oquam::assert_relative_speed(double relative_speed) 
        {
                if (relative_speed <= 0.0 || relative_speed > 1.0)
                        throw std::runtime_error("Oquam: invalid speed");
        }
        
        void Oquam::do_travel(Path &path, double relative_speed) 
        {
                r_debug("Oquam::travel_synchronized");
                
                assert_relative_speed(relative_speed); 
                
                double start_position[3];
                get_position(start_position);
                        
                Script script(start_position);

                double speed_ms = get_absolute_speed(relative_speed);
                
                build_script(path, speed_ms, script);
                convert_script(script, relative_speed);
                store_script(script);
                execute_script(script);
                wait_end_of_script(script); 
        }

        void Oquam::wait_end_of_script(Script& script) 
        {
                double duration = script_duration(script);
                double timeout = 60.0 + 1.5 * duration;
                synchronize(timeout);
        }
        
        void Oquam::synchronize(double timeout) 
        {
                if (!_controller->synchronize(timeout)) 
                        throw std::runtime_error("Oquam: synchronize failed");
        }

        void Oquam::build_script(Path &path, double speed, Script& script) 
        {
                for (size_t i = 0; i < path.size(); i++) {
                        double x = path[i].x;
                        double y = path[i].y;
                        double z = path[i].z;

                        if (valid_x(x) && valid_y(y) && valid_z(z)) {
                                script.moveto(x, y, z, speed);
                                
                        } else {
                                r_warn("Oquam: Point[%d]: out of bounds: "
                                       "(%0.4f, %0.4f, %0.4f)", x, y, z);
                                throw std::runtime_error("Point out of bounds");
                        }
                }
        }

        bool Oquam::get_position(int32_t *position) 
        {
                return _controller->get_position(position);
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

        double Oquam::script_duration(Script& script)
        {
                Section& section = script.slices.back();
                return section.at + section.duration;
        }

        void Oquam::convert_script(Script& script, double relative_speed) 
        {
                double vmax[3];
                smul(vmax, _vmax, relative_speed);
                
                r_debug("Oquam::convert_script: script_convert");
                
                script.convert(vmax, _amax, _path_max_deviation,
                               _path_slice_interval, 32.0);
                
                script.check_validity(32.0, _xmin, _xmax, vmax, _amax);
        }

        void Oquam::execute_move(Section& section, int32_t *pos_steps)
        {
                int32_t p1[3];
                p1[0] = (int32_t) (section.p1[0] * _scale_meters_to_steps[0]);
                p1[1] = (int32_t) (section.p1[1] * _scale_meters_to_steps[1]);
                p1[2] = (int32_t) (section.p1[2] * _scale_meters_to_steps[2]);
                
                int16_t dt = (int16_t) (1000.0 * section.duration);
                int16_t dx = (int16_t) (p1[0] - pos_steps[0]);
                int16_t dy = (int16_t) (p1[1] - pos_steps[1]);
                int16_t dz = (int16_t) (p1[2] - pos_steps[2]);
                
                if ((dt > 0) && (dx != 0 || dy != 0 || dz != 0)) {
                        bool success = _controller->move(dt, dx, dy, dz);
                        if (!success)
                                throw std::runtime_error("Oquam: move failed");
                        
                        // Update the current position
                        pos_steps[0] = p1[0];
                        pos_steps[1] = p1[1];
                        pos_steps[2] = p1[2];
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

        bool Oquam::stop_execution()
        {
                return _controller->stop_execution();
        }
        
        bool Oquam::continue_execution()
        {
                return _controller->continue_execution();
        }
        
        bool Oquam::reset()
        {
                return _controller->reset();
        }
}
