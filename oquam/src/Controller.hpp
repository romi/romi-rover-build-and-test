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
#ifndef _OQUAM_CONTROLLER_HPP_
#define _OQUAM_CONTROLLER_HPP_

#include "script.h"
#include "v.h"

class Controller
{
public:
        
        Controller(double *xmax, double *vmax, double *amax, double deviation)
                : _deviation(deviation), _script(0) {
                vcopy(_xmax, xmax);
                vcopy(_vmax, vmax);
                vcopy(_amax, amax);
        }
        
        virtual ~Controller() {}

        /**
         *  \brief Returns the current position.
         *
         *         Returns the current position in meters and radian. If the
         *         CNC is moving, the position is the last known position of
         *         the controller. In reality, the position is most likely
         *         different.
         */
        virtual int get_position(double *pos) = 0;

        /**
         *  \brief Run a script.  
         */
        virtual int run(script_t *script) = 0;

        /**
         *  \brief Continue the script.
         *
         *         The continue function should be called when the script
         *         started with controller_run is paused and should be
         *         resumed.
         */
        virtual int continue_script() = 0;

        /**
         *  \brief Move at a given speed.
         *
         *         Move at a given speed. The speed is given in m/s for the
         *         XYZ and radian/s for ABC.  
         */
        virtual int moveat(double *v) = 0;

        /**
         *  \brief Move to an absolute position.
         *
         *         The coordinates x, y, z are specified in meter. The speed
         *         is given in m/s.
         */
        virtual int moveto(double x, double y, double z, double v,
                           int move_x = 1, int move_y = 1, int move_z = 1) = 0;

        
        /* accessors */
        
        virtual const double *xmax() {
                return _xmax;
        }

        virtual const double *vmax() {
                return _vmax;
        }

        virtual const double *amax() {
                return _amax;
        }

        virtual double deviation() {
                return _deviation;
        }

        virtual script_t *get_script() {
                return _script;
        }

        virtual int valid_position(double value, int axis) {
                if (_xmax[axis] > 0.0)
                        return value >= 0 && value <= _xmax[axis];
                else
                        return value <= 0 && value >= _xmax[axis];
        }

        virtual int valid_x(double value) {
                return valid_position(value, 0);
        }

        virtual int valid_y(double value) {
                return valid_position(value, 1);
        }

        virtual int valid_z(double value) {
                return valid_position(value, 2);
        }
                
        virtual int valid_position(double *value) {
                return valid_position(value[0], 0)
                        && valid_position(value[1], 1)
                        && valid_position(value[2], 2);
        }

        virtual int valid_speed(double value, int axis) {
                return value >= 0 && value <= _vmax[axis];
        }

        virtual int valid_speed(double *value) {
                return valid_speed(value[0], 0)
                        && valid_speed(value[1], 1)
                        && valid_speed(value[2], 2);
        }

protected:
        
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
        double _deviation;

        /**
         * The current script being executed, if any.
         */        
        script_t *_script;
};

#endif // _OQUAM_CONTROLLER_HPP_
