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
#include "controller.h"

void delete_controller(controller_t *controller)
{
        if (controller->del)
                controller->del(controller);
}

int controller_position(controller_t *controller, double *pos)
{
        if (controller->position == NULL) {
                r_warn("Controller has no position function");
                return -1;
        }
        return controller->position(controller, pos);
}

int controller_moveat(controller_t *controller, double *v)
{
        if (controller->moveat == NULL) {
                r_warn("Controller has no moveat function");
                return -1;
        }
        return controller->moveat(controller, v);
}

int controller_run(controller_t *controller, cnc_t *cnc, planner_t *planner, int async)
{
        if (controller->run == NULL) {
                r_warn("Controller has no run function");
                return -1;
        }
        return controller->run(controller, cnc, planner, async);
}
