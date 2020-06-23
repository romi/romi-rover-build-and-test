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
#include "StepperController.hpp" 
#include "planner.h" 
#include <stdexcept>

StepperController::StepperController(double *xmax, double *vmax,
                                     double *amax, double deviation,
                                     double *scale, double interval)
        : Controller(xmax, vmax, amax, deviation),
          _interval(interval)
{
        vcopy(_scale, scale);
        
        _mutex = new_mutex();
        if (_mutex == 0)
                throw std::runtime_error("StepperController: new_mutex failed");

        for (int i = 0; i < 3; i++) {
                int32_t steps = _xmax[i] * _scale[i]; 
                if (steps < 0) {
                        r_err("Invalid dimensions: xmax and scale must have the same sign");
                        throw std::runtime_error("StepperController: Invalid values for xmax and/or scale");
                }
                if (steps > 32767) {
                        r_err("Invalid dimensions: "
                              "the workspace is too large (required steps %d > 32767)",
                                steps);
                        throw std::runtime_error("StepperController: Invalid dimensions");
                }
        }
}

StepperController::~StepperController()
{
        if (_mutex)
                delete_mutex(_mutex);
}

int StepperController::execute()
{
        for (int i = 0; i < _script->num_blocks; i++) {
                if (execute(_script->block + i) != 0)
                        return -1;
        }
        return 0;
}

int StepperController::wait()
{
        int errors = 0;
        
        while (1) {
                int r = is_busy();
                if (r == 0)
                        break;
                if (r == -1) {
                        r_warn("stepper_controller_run: is_busy returned an error");
                        errors++;
                        if (errors == 5)
                                return -1;
                }
                clock_sleep(0.200);
        }
        return 0;
}

int StepperController::run_locked(script_t *script)
{
        int err;
        double position[3];

        if (_script) {
                r_warn("stepper_controller_run: already executing a script");
                return -1;
        }

        _script = script;

        err = get_position(position);
        if (err != 0)
                goto return_error;

        script_clear(script);
        
        err = planner_convert_script(script, position, _vmax, _amax, _deviation);
        if (err != 0)
                goto return_error;

        err = planner_slice(script, _interval, 32.0);
        if (err != 0)
                goto return_error;
        
        err = compile();
        if (err != 0)
                goto return_error;
        
        err = execute();
        if (err != 0)
                goto return_error;
        
        err = wait();
        if (err != 0)
                goto return_error;

        _script = 0;
        return 0;

return_error:
        _script = 0;
        return -1;
}

int StepperController::run(script_t *script)
{
        mutex_lock(_mutex);
        int err = run_locked(script);
        mutex_unlock(_mutex);
        return err;
}

void StepperController::trigger(int id)
{
        script_do_trigger(_script, id);
}

int StepperController::push_wait()
{
        block_t block;
        block.type = BLOCK_WAIT;
        return script_push_block(_script, &block);
}

int StepperController::push_move(section_t *section,
                                 int32_t *pos_steps)
{
        block_t block;
        block.type = BLOCK_MOVE;

        int32_t p1[3];
        p1[0] = (int32_t) (section->p1[0] * _scale[0]);
        p1[1] = (int32_t) (section->p1[1] * _scale[1]);
        p1[2] = (int32_t) (section->p1[2] * _scale[2]);

        block.data[0] = (int16_t) (1000.0 * section->t);
        if (block.data[0] == 0)
                return 0;
        
        block.data[1] = (int16_t) (p1[0] - pos_steps[0]);
        block.data[2] = (int16_t) (p1[1] - pos_steps[1]);
        block.data[3] = (int16_t) (p1[2] - pos_steps[2]);
        if (block.data[1] == 0
            && block.data[2] == 0
            && block.data[3] == 0)
                return 0;
        
        block.id = _script->block_id++;
        block.action_id = section->id;
        
        // Keep the IDs reasonbly low so that the messages don't
        // become too long.
        if (_script->block_id == 10000) 
                _script->block_id = 0;
                
        pos_steps[0] = p1[0];
        pos_steps[1] = p1[1];
        pos_steps[2] = p1[2];
        
        return script_push_block(_script, &block);
}

int StepperController::push_delay(action_t *action)
{
        block_t block;
        uint32_t milliseconds;

        milliseconds = (uint32_t) (action->data.delay.duration * 1000.0f);        
        block.type = BLOCK_DELAY;
        
        while (milliseconds > 0) {
                if (milliseconds > 32767) {
                        block.data[0] = 32767;
                        milliseconds -= 32767;
                } else {
                        block.data[0] = milliseconds;
                        milliseconds = 0;
                }
                if (script_push_block(_script, &block) != 0)
                        return -1;
        }
        return 0;
}

int StepperController::push_trigger_action(action_t *action)
{
        block_t block;
        trigger_t trigger;
        uint32_t milliseconds;

        trigger.id = _script->num_triggers;
        trigger.arg = action->data.trigger.arg;
        trigger.callback = action->data.trigger.callback;
        trigger.userdata = action->data.trigger.userdata;
        trigger.delay = action->data.trigger.delay;

        milliseconds = (uint32_t) (action->data.trigger.delay * 1000.0f);        
        if (milliseconds > 32767) {
                r_warn("Trigger delay is being reduced from %d ms to 32767 ms",
                       milliseconds);
                milliseconds = 32767;
        }
        
        block.type = BLOCK_TRIGGER;
        block.data[0] = trigger.id;
        block.data[1] = milliseconds;
        
        if (script_push_trigger(_script, &trigger) != 0)
                return -1;

        if (script_push_block(_script, &block) != 0)
                return -1;
        
        return 0;
}

int StepperController::push_action(action_t *action)
{
        int err = 0;
        
        switch (action->type) {
        case ACTION_WAIT:
                err = push_wait();
                break;
        case ACTION_MOVE:
                r_err("Found move action while compiling");
                err = -1;
                break;
        case ACTION_DELAY:
                err = push_delay(action);
                break;
        case ACTION_TRIGGER:
                err = push_trigger_action(action);
                break;
        }
        
        return err;
}

int StepperController::push_actions(section_t *section)
{
        for (list_t *l = section->actions; l != 0; l = list_next(l)) {
                action_t *action = list_get(l, action_t);
                if (push_action(action) != 0)
                        return -1;
        }
        return 0;
}

int StepperController::compile()
{
        int err = -1;
        int32_t pos_steps[3];
        section_t *section = list_get(_script->slices, section_t);

        _script->block_id = 0;
        _script->num_blocks = 0;
        _script->num_triggers = 0;
        
        if (section) {
                pos_steps[0] = (int32_t) (section->p0[0] * _scale[0]);
                pos_steps[1] = (int32_t) (section->p0[1] * _scale[1]);
                pos_steps[2] = (int32_t) (section->p0[2] * _scale[2]);
        }
        
        for (list_t *l = _script->slices; l != 0; l = list_next(l)) {
                section = list_get(l, section_t);

                if (section->t > 0) {
                        if (push_move(section, pos_steps) != 0)
                                return -1;
                        
                } else if (section->actions) {
                        if (push_actions(section) != 0)
                                return -1;
                }
        }
        
        return 0;
}

