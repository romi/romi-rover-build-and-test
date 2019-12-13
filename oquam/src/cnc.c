#include <r.h>
#include "cnc.h"
#include "planner.h"
#include "controller.h"

struct _cnc_t {
        controller_t *controller;
        planner_t *planner;
        script_t *script;
};

cnc_t *new_cnc(controller_t *controller)
{
        cnc_t *cnc = r_new(cnc_t);
        if (cnc == NULL)
                return NULL;
        cnc->controller = controller;
        return cnc;
}

void delete_cnc(cnc_t *cnc)
{
        if (cnc) {
                if (cnc->script != NULL)
                        delete_script(cnc->script);
                if (cnc->planner != NULL)
                        delete_planner(cnc->planner);
                r_delete(cnc);
        }
}

int cnc_begin_script(cnc_t *cnc, double d)
{
        if (cnc->script != NULL) {
                delete_script(cnc->script);
                cnc->script = NULL;
        }
        cnc->script = new_script(d);
        if (cnc->script == NULL)
                return -1;
        return 0;
}

int cnc_move(cnc_t *cnc, double x, double y, double z, double v)
{
        if (cnc->script == NULL) {
                r_warn("cnc_moveto: no script initiated");
                return -1;
        }
        if (v <= 0.0f) {
                r_warn("cnc_moveto: speed must be positive");
                return -1;
        }
                
        action_t *action = new_move(x, y, z, v);
        if (action == NULL)
                return -1;
        
        script_append(cnc->script, action);
        return 0;
}

int cnc_delay(cnc_t *cnc, double seconds)
{        
        if (cnc->script == NULL) {
                r_warn("cnc_moveto: no script initiated");
                return -1;
        }
        action_t *action = new_delay(seconds);
        if (action == NULL)
                return -1;
        
        script_append(cnc->script, action);
        return 0;
}

int cnc_trigger(cnc_t *cnc, cnc_callback_t cb, void *userdata, int arg)
{        
        if (cnc->script == NULL) {
                r_warn("cnc_moveto: no script initiated");
                return -1;
        }
        action_t *action = new_trigger(cb, userdata, arg);
        if (action == NULL)
                return -1;
        
        script_append(cnc->script, action);
        return 0;
}

int cnc_end_script(cnc_t *cnc)
{
        if (cnc->script == NULL) {
                r_warn("cnc_moveto: no script initiated");
                return -1;
        }

        if (cnc->planner != NULL) {
                delete_planner(cnc->planner);
        }
        
        cnc->planner = new_planner(cnc, cnc->controller, cnc->script);
        if (cnc->planner == NULL)
                return -1;
        
        return 0;
}

int cnc_run_script(cnc_t *cnc, int async)
{
        if (cnc->planner == NULL) {
                r_warn("cnc_run_script: no planner initiated. "
                       "Did you call cnc_end_script?");
                return -1;
        }
        return controller_run(cnc->controller, cnc, cnc->planner, async);
}

int cnc_get_position(cnc_t *cnc, double *p)
{
        return controller_position(cnc->controller, p);
}

/* int cnc_plot_script(cnc_t *cnc, const char *filepath) */
/* { */
/*         if (cnc->planner == NULL) { */
/*                 r_warn("No script"); */
/*                 return -1; */
/*         } */
/*         return planner_plot(cnc->planner, filepath); */
/* } */
