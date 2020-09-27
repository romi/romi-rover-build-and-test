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
#include "workspace.h"

int workspace_parse(workspace_t *workspace, json_object_t w)
{
        if (json_falsy(w) || !json_isarray(w) || json_array_length(w) != 4) {
                r_err("Invalid workspace configuration");
                return -1;
        }
        
        double x0 = json_array_getnum(w, 0);
        double y0 = json_array_getnum(w, 1);
        double width = json_array_getnum(w, 2);
        double height = json_array_getnum(w, 3);
        if (isnan(x0) || isnan(y0)
            || isnan(width) || isnan(height)) {
                r_err("Invalid workspace values");
                return -1;
        }

        workspace->x0 = (int) x0;
        workspace->y0 = (int) y0;
        workspace->width = (int) width;
        workspace->height = (int) height;

        r_debug("workspace: x0 %.0f, y0 %.0f, width %.0f px, height %.0f px", 
                x0, y0, width, height);

        return 0;
}

json_object_t workspace_to_json(workspace_t *workspace)
{
        json_object_t w = json_array_create();
        json_array_setnum(w, workspace->x0, 0);
        json_array_setnum(w, workspace->y0, 1);
        json_array_setnum(w, workspace->width, 2);
        json_array_setnum(w, workspace->height, 3);
        return w;
}

