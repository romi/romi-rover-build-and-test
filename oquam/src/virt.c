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
        double period;        
        char *output;
        double step[3];
} virtual_stepper_controller_t;

static void delete_virtual_controller(controller_t *controller);
static int virtual_stepper_controller_run(controller_t *controller, cnc_t *cnc,
                                          planner_t *planner, int async);
static int virtual_stepper_controller_position(controller_t *controller, double *p);
static int virtual_stepper_controller_moveat(controller_t *controller, double *v);
static int virtual_stepper_controller_execute(virtual_stepper_controller_t* controller,
                                              block_t *block, int len);


controller_t *new_virtual_stepper_controller(const char *output,
                                             double *xmax, double *vmax, double *amax,
                                             double *scale, double period)
{
        virtual_stepper_controller_t *controller = r_new(virtual_stepper_controller_t);
        if (controller == NULL)
                return NULL;
        
        controller->period = period;        
        vcopy(controller->stepper.interface.xmax, xmax);
        vcopy(controller->stepper.interface.vmax, vmax);
        vcopy(controller->stepper.interface.amax, amax);
        vcopy(controller->stepper.scale, scale);

        controller->stepper.interface.run = virtual_stepper_controller_run;
        controller->stepper.interface.position = virtual_stepper_controller_position;
        controller->stepper.interface.moveat = virtual_stepper_controller_moveat;
        controller->stepper.interface.del = delete_virtual_controller;
        controller->stepper.execute = (stepper_controller_execute_blocks_t) virtual_stepper_controller_execute;

        vzero(controller->step);
        
        ////

        if (stepper_controller_init(&controller->stepper) != 0) {
                delete_virtual_controller((controller_t *)controller);
                return NULL;
        }

        controller->output = output? r_strdup(output) : NULL;
        
        return (controller_t*) controller;
}

void delete_virtual_controller(controller_t *controller)
{
        virtual_stepper_controller_t *c = (virtual_stepper_controller_t*) controller;
        if (c) {
                stepper_controller_cleanup((stepper_controller_t*) controller);
                if (c->output) 
                        r_free(c->output);
                r_delete(c);
        }
}

static int virtual_stepper_controller_execute(virtual_stepper_controller_t* controller,
                                              block_t *block, int len)
{
        double dt;
        
        char filename[1024];
        rprintf(filename, sizeof(filename), "%s.txt", controller->output);

        FILE *fp = fopen(filename, "w");
        if (fp == NULL) {
                r_err("Failed to open file: %s", filename);
                return -1;
        }
        
        for (int i = 0; i < len; i++) {
                switch (block[i].type) {
                case BLOCK_WAIT:
                        fprintf(fp, "W\n");
                        printf("W\n");
                        break;
                        
                case BLOCK_MOVE:
                        fprintf(fp, "M[%d,%d,%d,%d]\n",
                               block[i].data[0],
                               block[i].data[1],
                               block[i].data[2],
                               block[i].data[3]);
                        printf("M[%d,%d,%d,%d]\n",
                               block[i].data[0],
                               block[i].data[1],
                               block[i].data[2],
                               block[i].data[3]);
                        dt = (double) block[i].data[0] / 1000.0;
                        clock_sleep(dt);
                        controller->step[0] += block[i].data[1];
                        controller->step[1] += block[i].data[2];
                        controller->step[2] += block[i].data[3];
                        break;
                        
                case BLOCK_DELAY:
                        fprintf(fp, "D%d\n", block[i].data[0]);
                        printf("D%d\n", block[i].data[0]);
                        dt = (double) block[i].data[0] / 1000.0;
                        clock_sleep(dt);
                        break;
                        
                case BLOCK_TRIGGER:
                        fprintf(fp, "T%d\n", block[i].data[0]);
                        printf("T%d\n", block[i].data[0]);
                        stepper_controller_do_trigger(&controller->stepper, block[i].data[0]);
                        break;
                        
                default:
                        fprintf(fp, "ERR Unknown block type\n");
                        break;
                }
        }
        
        fclose(fp);
        
        return 0;
}

static int virtual_stepper_controller_run(controller_t* controller, cnc_t *cnc,
                                          planner_t *planner, int async)
{
        stepper_controller_t *stepper = (stepper_controller_t*) controller;
        virtual_stepper_controller_t *c = (virtual_stepper_controller_t*) controller;

        list_t *slices = planner_slice(planner, c->period, 32.0);
        if (slices == NULL)
                return -1;
        
        if (stepper_controller_compile(stepper, slices, cnc, planner) != 0)
                return -1;

        if (c->output) {
                char svg[1024];
                rprintf(svg, sizeof(svg), "%s.svg", c->output);
                
                print_paths(svg,
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
        vdiv(p, c->step, c->stepper.scale);        
        return 0;
}

static int virtual_stepper_controller_moveat(controller_t* controller, double *v)
{
        virtual_stepper_controller_t *c = (virtual_stepper_controller_t*) controller;
        r_err("virtual_stepper_controller_moveat: not implemented");
        return -1;
}
