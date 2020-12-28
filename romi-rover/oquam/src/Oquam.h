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
#ifndef __ROMI_OQUAM_H
#define __ROMI_OQUAM_H

#include <r.h>
#include <mutex>
#include "v.h"
#include "CNC.h"
#include "ICNCController.h"
#include "IFileCabinet.h"
#include "Script.h" 

namespace romi {

        using SynchonizedCodeBlock = std::lock_guard<std::mutex>;
        
        class Oquam : public CNC
        {
        public:
                CNCRange _range;
                ICNCController *_controller;
                IFileCabinet *_file_cabinet;
                std::mutex _m;
        
                /**
                 * The minumum positions, in m
                 */
                double _xmin[3];
                
                /**
                 * The maximum positions, in m
                 */
                double _xmax[3];

                /**
                 * The maximum speed, in m/s
                 */
                double _vmax[3];

                /**
                 * The maximum acceleration, in m/s^2
                 */
                double _amax[3];

                /**
                 * The maximum deviation allowed when computed a continuous
                 * path, in m.
                 */        
                double _path_max_deviation;

                double _scale_meters_to_steps[3];
                
                double _path_slice_interval;

                int _script_count;

                
        public:
                Oquam(ICNCController *controller,
                      const double *xmin, const double *xmax,
                      const double *vmax, const double *amax,
                      const double *scale_meters_to_steps, 
                      double path_max_deviation,
                      double path_slice_interval);
                
                virtual ~Oquam() = default;

                void set_file_cabinet(IFileCabinet *cabinet) {
                        _file_cabinet = cabinet;
                }
                
                bool get_range(CNCRange &range) {
                        range._x[0] = _xmin[0];
                        range._x[1] = _xmax[0];
                        range._y[0] = _xmin[1];
                        range._y[1] = _xmax[1];
                        range._z[0] = _xmin[2];
                        range._z[1] = _xmax[2];
                        return true;
                }

                // See ICNC.h for more info
                bool moveto(double x, double y, double z,
                            double relative_speed = 0.1) override {
                        SynchonizedCodeBlock synchronize(_m);
                        return moveto_synchronized(x, y, z, relative_speed);
                }
                
                bool travel(Path &path, double relative_speed = 0.1) override {
                        SynchonizedCodeBlock synchronize(_m);
                        return travel_synchronized(path, relative_speed);
                }
                
                bool spindle(double speed) override {
                        // TODO
                        r_err("Oquam::spindle NOT IMPLEMENTED YET!");
                        return true;
                }

                bool homing() override {
                        SynchonizedCodeBlock synchronize(_m);
                        return _controller->homing();
                }

                bool stop_execution() override;
                bool continue_execution() override;
                bool reset() override;
                
                /* accessors */
        
                virtual const double *xmin() {
                        return _xmin;
                }
        
                virtual const double *xmax() {
                        return _xmax;
                }

                virtual const double *vmax() {
                        return _vmax;
                }

                virtual const double *amax() {
                        return _amax;
                }

                virtual double path_max_deviation() {
                        return _path_max_deviation;
                }

                virtual bool valid_position(double value, int axis) {
                        return value >= _xmin[axis] && value <= _xmax[axis];
                }

                virtual bool valid_x(double value) {
                        return valid_position(value, 0);
                }

                virtual bool valid_y(double value) {
                        return valid_position(value, 1);
                }

                virtual bool valid_z(double value) {
                        return valid_position(value, 2);
                }

        protected:
                bool moveto_synchronized(double x, double y, double z, double rel_speed);
                bool travel_synchronized(Path &path, double relative_speed);

                bool get_position(double *position); 
                bool get_position(int32_t *position); 
                Script *build_script(Path &path, double speed); 
                bool convert_script(Script *script, double *position, double rel_speed); 
                bool execute_script(Script *script);
                bool execute_move(Section *section, int32_t *pos_steps);
                double script_duration(Script *script);
                void store_script(Script *script);
        };
}

#endif // __ROMI_OQUAM_H
