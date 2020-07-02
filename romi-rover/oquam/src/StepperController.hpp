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

#ifndef _OQUAM_STEPPERCONTROLLER_HPP_
#define _OQUAM_STEPPERCONTROLLER_HPP_

#include "Controller.hpp" 

class StepperController : public Controller
{
public:
        
        StepperController(double *xmax, double *vmax, double *amax,
                          double deviation, double *scale, double interval);
        
        virtual ~StepperController();

        virtual const double *scale() {
                return _scale;
        }

        virtual double interval() {
                return _interval;
        }

        virtual int run(script_t *script);

        virtual int execute(block_t *block) = 0;

        virtual int is_busy() = 0;

protected:

        /** 
         * The scale converts a distance in meters to the
         * corresponding number of steps, steps = meters x scale, 
         * or meters = steps/scale.
         */
        double _scale[3];

        /** 
         * The time interval used to slice a script's path into short
         * move actions.
         */
        double _interval;

        mutex_t *_mutex;
        
        int run_locked(script_t *script);

        int execute();
        
        int wait();

        void trigger(int id);

        int compile();
        int push_wait();
        int push_move(section_t *section, int32_t *pos_steps);
        int push_delay(action_t *action);
        int push_trigger_action(action_t *action);
        int push_action(action_t *action);
        int push_actions(section_t *section);
};

#endif // _OQUAM_STEPPERCONTROLLER_HPP_
