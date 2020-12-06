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
#include "script.h"
#include "planner.h"
#include "plotter.h"
#include "Oquam.h"

namespace romi {

        void Oquam::moveto_synchronized(double x, double y, double z, double rel_speed)
        {
                if (rel_speed > 0.0 && rel_speed <= 1.0) {
                        
                        double position[3];
                        get_position(position);
                
                        double dx[3];
                        dx[0] = x - position[0];
                        dx[1] = y - position[1];
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
                                
                                _controller.move(millis, step_x, step_y, step_z);
                                _controller.synchronize(2.0 * duration);
                                
                        }
                } else {
                        throw std::runtime_error("Invalid speed");
                }
        }

        void Oquam::store_script(script_t *script) 
        {
                if (_file_cabinet) {
                        IFolder &folder = _file_cabinet->get_current_folder();
                        membuf_t *svg = plot_to_mem(script, _xmin, _xmax, _vmax,
                                                    _amax, _scale_meters_to_steps);
                        if (svg != 0) {
                                folder.store_svg("path", membuf_data(svg),
                                                 membuf_len(svg));
                                delete_membuf(svg);
                        } else {
                                r_warn("Oquam::store_script: plot failed");
                        }
                }
        }
        
        void Oquam::travel_synchronized(Path &path, double relative_speed) 
        {
                r_debug("Oquam::travel_synchronized");
                
                if (relative_speed <= 0.0 || relative_speed > 1.0)
                        throw std::runtime_error("Invalid speed");
                
                double position[3];
                get_position(position);
                
                // FIXME
                double v[3];
                smul(v, _vmax, relative_speed);
                double speed_ms = ::vmax(v);
                
                script_t *script = build_script(path, speed_ms);
                        
                if (script != 0) {
                                
                        if (convert_script(script, position, relative_speed)) {

                                store_script(script);
                                
                                if (execute_script(script)) {
                                                
                                        double duration = script_duration(script);
                                        double timeout = 60.0 + 1.5 * duration;
                                                
                                        delete_script(script);
                                        
                                        if (!_controller.synchronize(timeout)) 
                                                throw std::runtime_error("Time out");
                                                
                                } else {
                                        delete_script(script);
                                        throw std::runtime_error("execute_script failed");
                                }

                        } else {
                                delete_script(script);
                                throw std::runtime_error("convert_script failed");
                        }
                                
                } else {
                        throw std::runtime_error("build_script failed");
                }
        }

        
        script_t *Oquam::build_script(Path &path, double speed) 
        {
                r_debug("new_script");
                script_t *script = new_script();

                for (size_t i = 0; i < path.size(); i++) {
                        double x = path[i].x;
                        double y = path[i].y;
                        double z = path[i].z;

                        if (valid_x(x) && valid_y(y) && valid_z(z)) {
                                script_moveto(script, x, y, z, speed);
                        } else {
                                r_warn("Oquam::build_script: Point[%d]: out of bounds: "
                                       "(%0.4f, %0.4f, %0.4f)", x, y, z);
                                delete_script(script);
                                script = 0;
                                break;
                        }
                }
                return script;
        }
        
        void Oquam::get_position(int32_t *position) 
        {
                if (!_controller.get_position(position)) {
                        r_err("controller.get_position failed");
                        throw std::runtime_error("controller.get_position failed");
                }
        }
        
        void Oquam::get_position(double *position) 
        {
                int32_t p[3];
                get_position(p); 
                for (int i = 0; i < 3; i++)
                        position[i] = (double) p[i] / _scale_meters_to_steps[i];
        }

        double Oquam::script_duration(script_t *script)
        {
                double duration = 0.0;
                for (list_t *l = script->slices; l != 0; l = list_next(l)) {
                        section_t *section = list_get(l, section_t);
                        duration = section->at + section->t; // We only need the last one..
                }
                return duration;
        }

        bool Oquam::convert_script(script_t *script, double *position,
                                   double relative_speed) 
        {
                bool success = false;
                double vmax[3];
                smul(vmax, _vmax, relative_speed);
                
                r_debug("Oquam::convert_script: planner_convert_script");
                if (planner_convert_script(script, position, _xmin, _xmax,
                                           vmax, _amax, _path_max_deviation) == 0) {
                        
                        r_debug("Oquam::convert_script: planner_slice");
                        if (planner_slice(script, _path_slice_interval, 32.0) == 0) {
                                success = true;
                        } else {
                                r_err("planner_slice failed");
                        }
                } else {
                        r_err("planner_convert_script failed");
                }
                
                return success;
        }
        
        bool Oquam::execute_move(section_t *section, int32_t *pos_steps)
        {
                bool success = true;
                int32_t p1[3];
                p1[0] = (int32_t) (section->p1[0] * _scale_meters_to_steps[0]);
                p1[1] = (int32_t) (section->p1[1] * _scale_meters_to_steps[1]);
                p1[2] = (int32_t) (section->p1[2] * _scale_meters_to_steps[2]);
                
                int16_t dt = (int16_t) (1000.0 * section->t);
                int16_t dx = (int16_t) (p1[0] - pos_steps[0]);
                int16_t dy = (int16_t) (p1[1] - pos_steps[1]);
                int16_t dz = (int16_t) (p1[2] - pos_steps[2]);

                r_debug("Abs(%.3f,%.3f,%.3f)=(%d,%d,%d) - D(%d,%d,%d)",
                        section->p1[0], section->p1[1], section->p1[2],
                        p1[0], p1[1], p1[2],
                        dx, dy, dz);
                
                if ((dt > 0) && (dx != 0 || dy != 0 || dz != 0)) {
                        success = _controller.move(dt, dx, dy, dz);
                        
                        // Update the current position
                        // pos_steps[0] = p1[0];
                        // pos_steps[1] = p1[1];
                        // pos_steps[2] = p1[2];
                        
                        // Update the current position
                        pos_steps[0] += dx;
                        pos_steps[1] += dy;
                        pos_steps[2] += dz;

                        {
                                _controller.synchronize(10.0);
                                int32_t position[3];
                                get_position(position);
                                r_debug("Pos(%d,%d,%d)",
                                        position[0], position[1], position[2]);
                        }
                        
                        
                } else {
                        r_debug("not (dt > 0) && (dx != 0 || dy != 0 || dz != 0): "
                                "dt=%d dx=%d dy=%d dz=%d", dt, dx, dy, dz);
                }

                return success;
        }
        
        bool Oquam::execute_script(script_t *script) 
        {
                bool success = true;
                int32_t pos_steps[3];
                section_t *section = list_get(script->slices, section_t);
                double *scale = _scale_meters_to_steps;
                
                if (section) {
                        // Initialize the start position
                        pos_steps[0] = (int32_t) (section->p0[0] * scale[0]);
                        pos_steps[1] = (int32_t) (section->p0[1] * scale[1]);
                        pos_steps[2] = (int32_t) (section->p0[2] * scale[2]);

                        r_debug("Start: Abs(%.3f,%.3f,%.3f)=(%d,%d,%d)",
                                section->p0[0], section->p0[1], section->p0[2],
                                pos_steps[0], pos_steps[1], pos_steps[2]);

                        int count = 0;
                        for (list_t *l = script->slices; l != 0; l = list_next(l)) {
                                r_debug("Section %d:", count);
                                section = list_get(l, section_t);
                                if (!execute_move(section, pos_steps)) {
                                        success = false;
                                        break;
                                }
                                count++;
                        }

                        r_debug("End: Abs(%d,%d,%d)",
                                pos_steps[0], pos_steps[1], pos_steps[2]);
                }
                return success;
        }

        void Oquam::stop_execution()
        {
                if (!_controller.stop_execution())
                        throw std::runtime_error("Homing failed");
        }
        
        void Oquam::continue_execution()
        {
                if (!_controller.continue_execution())
                        throw std::runtime_error("Continue failed");
        }
        
        void Oquam::reset()
        {
                if (!_controller.reset())
                        throw std::runtime_error("Reset failed");
        }
}
