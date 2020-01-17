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
#include "script.h"

typedef struct _script_t {
        double d;
        list_t *actions;
} script_t;

script_t *new_script(double d)
{
        script_t *script = r_new(script_t);
        if (script == NULL)
                return NULL;
        script->d = d;
        return script;
}

void delete_script(script_t *script)
{
        if (script) {
                for (list_t *l = script->actions; l; l = list_next(l)) {
                        action_t *n = list_get(l, action_t);
                        delete_action(n);
                }
                delete_list(script->actions);
                r_delete(script);
        }
}

void script_append(script_t *script, action_t *action)
{
        script->actions = list_append(script->actions, action);
}

list_t *script_actions(script_t *script)
{
        return script->actions;
}

double script_deviation(script_t *script)
{
        return script->d;
}
