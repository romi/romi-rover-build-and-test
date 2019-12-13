#include <r.h>
#include "cnc.h"
#include "v.h"
#include "stepper.h"
#include "planner.h"
#include "plotter.h"
#include "virt.h"

/**************************************************************************/

typedef struct _virtual_stepper_controller_t {
        stepper_controller_t stepper;
        
        char *output;
        char *svg;
        
} virtual_stepper_controller_t;

static void delete_virtual_controller(controller_t *controller);
static int virtual_stepper_controller_run(controller_t *controller, cnc_t *cnc,
                                          planner_t *planner, int async);
static int virtual_stepper_controller_position(controller_t *controller, double *p);
static int virtual_stepper_controller_moveat(controller_t *controller, double *v);
static int virtual_stepper_controller_execute(virtual_stepper_controller_t* controller,
                                              block_t *block, int len);


controller_t *new_virtual_stepper_controller(const char *output, const char *svg)
{
        virtual_stepper_controller_t *controller = r_new(virtual_stepper_controller_t);
        if (controller == NULL)
                return NULL;

        controller->stepper.interface.xmax[0] = 0.7; // m
        controller->stepper.interface.xmax[1] = 0.7;
        controller->stepper.interface.xmax[2] = 0.4;

        // vmax: x, y: 1200 revolutions/minute x 0.020 m/revolutions / 60 s/minute
        controller->stepper.interface.vmax[0] = 0.020 * 1200.0 / 60.0; // m/s
        controller->stepper.interface.vmax[1] = 0.020 * 1200.0 / 60.0;
        // vmax: z:  1200 revolutions/minute x 0.002 m/revolutions / 60 s/minute
        controller->stepper.interface.vmax[2] = 0.002 * 1200.0 / 60.0;

        // TODO
        controller->stepper.interface.amax[0] = 1.0f; // m/s²
        controller->stepper.interface.amax[1] = 1.0f;
        controller->stepper.interface.amax[2] = 0.1f;

        controller->stepper.interface.run = virtual_stepper_controller_run;
        controller->stepper.interface.position = virtual_stepper_controller_position;
        controller->stepper.interface.moveat = virtual_stepper_controller_moveat;
        controller->stepper.interface.del = delete_virtual_controller;

        ////

        if (stepper_controller_init(&controller->stepper) != 0) {
                delete_virtual_controller((controller_t *)controller);
                return NULL;
        }

        controller->stepper.execute = (stepper_controller_execute_blocks_t)
                virtual_stepper_controller_execute;
        
        // scale: x, y: 200 steps -> 0.02 m => 1m -> 10000 steps => 1 step = 0.1 mm
        controller->stepper.scale[0] = 10000.0f;
        controller->stepper.scale[1] = 10000.0f;
        // scale: z: 200 steps -> 0.002 m => 1m -> 100000 steps 
        controller->stepper.scale[2] = 100000.0f;

        ////
                
        controller->output = output? r_strdup(output) : NULL;
        controller->svg = svg? r_strdup(svg) : NULL;
        
        return (controller_t*) controller;
}

void delete_virtual_controller(controller_t *controller)
{
        virtual_stepper_controller_t *c = (virtual_stepper_controller_t*) controller;
        if (c) {
                stepper_controller_cleanup((stepper_controller_t*) controller);
                if (c->output) 
                        r_free(c->output);
                if (c->svg) 
                        r_free(c->svg);
                r_delete(c);
        }
}

static int virtual_stepper_controller_execute(virtual_stepper_controller_t* controller,
                                              block_t *block, int len)
{
        for (int i = 0; i < len; i++) {
                switch (block[i].type) {
                case BLOCK_WAIT:
                        printf("W\n");
                        break;
                case BLOCK_MOVE:
                        printf("M[%d,%d,%d,%d]\n",
                               block[i].data[0],
                               block[i].data[1],
                               block[i].data[2],
                               block[i].data[3]);
                        break;
                case BLOCK_DELAY:
                        printf("D%d\n", block[i].data[0]);
                        break;
                case BLOCK_TRIGGER:
                        printf("T%d\n", block[i].data[0]);
                        break;
                case BLOCK_BEGIN:
                        printf("B%d\n", block[i].data[0]);
                        break;
                case BLOCK_FINISHED:
                        printf("Q\n");
                        break;
                default:
                        printf("ERR Unknown block type\n");
                        break;
                }
        }
}

static int virtual_stepper_controller_run(controller_t* controller, cnc_t *cnc,
                                          planner_t *planner, int async)
{
        stepper_controller_t *stepper = (stepper_controller_t*) controller;
        virtual_stepper_controller_t *c = (virtual_stepper_controller_t*) controller;

        list_t *slices = planner_slice(planner, 0.010, 32.0);
        if (slices == NULL)
                return -1;
        
        if (stepper_controller_compile(stepper, slices, cnc, planner) != 0)
                return -1;

        if (c->svg) {
                print_paths(c->svg,
                            planner_get_segments_list(planner),
                            planner_get_atdc_list(planner),
                            slices,
                            stepper->block,
                            stepper->num_blocks,
                            stepper->interface.xmax,
                            stepper->interface.vmax,
                            stepper->interface.amax,
                            stepper->scale);
        }
        
        if (stepper_controller_execute(stepper, async) != 0)
                return -1;
        
        return 0;
}

static int virtual_stepper_controller_position(controller_t* controller, double *p)
{
        virtual_stepper_controller_t *c = (virtual_stepper_controller_t*) controller;
        r_warn("virtual_stepper_controller_position: not implemented");
        vzero(p);
        return 0;
}

static int virtual_stepper_controller_moveat(controller_t* controller, double *v)
{
        virtual_stepper_controller_t *c = (virtual_stepper_controller_t*) controller;
        r_err("virtual_stepper_controller_moveat: not implemented");
        return -1;
}
