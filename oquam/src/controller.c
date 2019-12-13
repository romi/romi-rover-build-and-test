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
