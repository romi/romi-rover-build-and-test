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
#include <r.h>
#include "action.h"

action_t *new_action(int type)
{
        action_t *action = r_new(action_t);
        if (action == NULL)
                return NULL;
        action->type = type;
        return action;
}

action_t *action_clone(action_t *a)
{
        action_t *action = r_new(action_t);
        if (action == NULL)
                return NULL;
        memcpy(action, a, sizeof(action_t));
        return action;
}

action_t *new_wait()
{
        action_t *action = new_action(ACTION_WAIT);
        if (action == NULL)
                return NULL;
        return action;
}

action_t *new_delay(double delay)
{
        action_t *action = new_action(ACTION_DELAY);
        if (action == NULL)
                return NULL;
        action->delay = delay;
        return action;
}

action_t *new_move(double x, double y, double z, double v)
{
        action_t *action = new_action(ACTION_MOVE);
        if (action == NULL)
                return NULL;
        action->p[0] = x;
        action->p[1] = y;
        action->p[2] = z;
        action->v = v;
        return action;
}

action_t *new_trigger(void *callback, void *userdata, int arg)
{
        action_t *action = new_action(ACTION_TRIGGER);
        if (action == NULL)
                return NULL;
        action->callback = callback;
        action->userdata = userdata;
        action->arg = arg;
        return action;
}

void delete_action(action_t *action)
{
        if (action) {
                r_delete(action);
        }
}
