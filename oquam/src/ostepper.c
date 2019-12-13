#include <r.h>
#include "cnc.h"
#include "v.h"
#include "stepper.h"
#include "planner.h"
#include "plotter.h"
#include "ostepper.h"

/**************************************************************************/

typedef struct _oquam_stepper_controller_t {
        stepper_controller_t stepper;        
        double period;
        membuf_t *buffer;
        char *device;
        serial_t *serial;
        
} oquam_stepper_controller_t;

static void delete_oquam_stepper_controller(controller_t *controller);
static int oquam_stepper_controller_run(controller_t *controller, cnc_t *cnc,
                                          planner_t *planner, int async);
static int oquam_stepper_controller_position(controller_t *controller, double *p);
static int oquam_stepper_controller_moveat(controller_t *controller, double *v);
static int oquam_stepper_controller_execute(oquam_stepper_controller_t* controller,
                                              block_t *block, int len);


controller_t *new_oquam_stepper_controller(const char *device,
                                           double *xmax, double *vmax, double *amax,
                                           double *scale, double period)
{
        oquam_stepper_controller_t *controller = r_new(oquam_stepper_controller_t);
        if (controller == NULL)
                return NULL;

        controller->period = period;
        
        controller->buffer = new_membuf();;
        if (controller->buffer == NULL) {
                delete_oquam_stepper_controller((controller_t *)controller);
                return NULL;
        }

        controller->device = r_strdup(device);
        if (controller->device == NULL) {
                delete_oquam_stepper_controller((controller_t *)controller);
                return NULL;
        }
        
        controller->serial = new_serial(device, 115200, 1);
        if (controller->serial == NULL) {
                delete_oquam_stepper_controller((controller_t *)controller);
                return NULL;
        }
        
        vcopy(controller->stepper.interface.xmax, xmax);
        vcopy(controller->stepper.interface.vmax, vmax);
        vcopy(controller->stepper.interface.amax, amax);

        controller->stepper.interface.run = oquam_stepper_controller_run;
        controller->stepper.interface.position = oquam_stepper_controller_position;
        controller->stepper.interface.moveat = oquam_stepper_controller_moveat;
        controller->stepper.interface.del = delete_oquam_stepper_controller;

        ////

        if (stepper_controller_init(&controller->stepper) != 0) {
                delete_oquam_stepper_controller((controller_t *)controller);
                return NULL;
        }

        controller->stepper.execute = (stepper_controller_execute_blocks_t)
                oquam_stepper_controller_execute;
        
        vcopy(controller->stepper.scale, scale);
        
        return (controller_t*) controller;
}

void delete_oquam_stepper_controller(controller_t *controller)
{
        oquam_stepper_controller_t *c = (oquam_stepper_controller_t*) controller;
        if (c) {
                stepper_controller_cleanup((stepper_controller_t*) controller);
                if (c->serial)
                        delete_serial(c->serial);
                if (c->device)
                        r_free(c->device);
                if (c->buffer)
                        delete_membuf(c->buffer);
                r_delete(c);
        }
}

static int oquam_stepper_controller_run(controller_t* controller, cnc_t *cnc,
                                        planner_t *planner, int async)
{
        stepper_controller_t *stepper = (stepper_controller_t*) controller;
        oquam_stepper_controller_t *c = (oquam_stepper_controller_t*) controller;

        list_t *slices = planner_slice(planner, c->period, 32.0);
        if (slices == NULL)
                return -1;
        
        if (stepper_controller_compile(stepper, slices, cnc, planner) != 0)
                return -1;

        if (0) {
                print_paths("oquam-stepper.svg",
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

static int oquam_stepper_controller_send_command(oquam_stepper_controller_t* controller,
                                                 const char *cmd)
{
        membuf_clear(controller->buffer);

        r_debug("%s", cmd);
        
        const char *r = serial_command_send(controller->serial,
                                            controller->buffer,
                                            cmd);
        r_debug("%s -> %s", cmd, r);
        
        if (strncmp(r, "RE", 2) == 0)
                return 1;
        if (strncmp(r, "OK", 2) == 0)
                return 0;
        if (strncmp(r, "ERR", 3) == 0) {
                r_err("oquam_stepper_controller_send_command: %s", r);
                return -1;
        }
        return -1;
}

static int oquam_stepper_controller_send_block(oquam_stepper_controller_t* controller,
                                               block_t *block)
{
        char cmd[64];
        
        switch (block->type) {
        case BLOCK_WAIT:
                rprintf(cmd, sizeof(cmd), "W\0");
                break;
        case BLOCK_MOVE:
                rprintf(cmd, sizeof(cmd), "M[%d,%d,%d,%d,%d]\0",
                       block->data[0],
                       block->data[1],
                       block->data[2],
                       block->data[3],
                       block->id);
                break;
        case BLOCK_DELAY:
                rprintf(cmd, sizeof(cmd), "D%d", block->data[0]);
                break;
        case BLOCK_TRIGGER:
                rprintf(cmd, sizeof(cmd), "T%d", block->data[0]);
                break;
        case BLOCK_BEGIN:
                rprintf(cmd, sizeof(cmd), "B%d", block->data[0]);
                break;
        case BLOCK_FINISHED:
                rprintf(cmd, sizeof(cmd), "Q");
                break;
        default:
                r_err("oquam_stepper_controller_send_block: Unknown block type");
                return -1;
        }

        while (1) {
                int err = oquam_stepper_controller_send_command(controller, cmd);
                if (err == 0)
                        return 0;
                if (err < 0)
                        return err;
                if (err > 0)
                        clock_sleep(1.0);
        }
        
        return 0;
}

static int oquam_stepper_controller_execute(oquam_stepper_controller_t* controller,
                                            block_t *block, int len)
{
        for (int i = 0; i < len; i++) {
                int err = oquam_stepper_controller_send_block(controller, block + i);
                if (err != 0)
                        return err;
        }
        return 0;
}

static int oquam_stepper_controller_position(controller_t* controller, double *p)
{
        oquam_stepper_controller_t *c = (oquam_stepper_controller_t*) controller;
        r_warn("oquam_stepper_controller_position: not implemented");
        vzero(p);
        return 0;
}

static int oquam_stepper_controller_moveat(controller_t* controller, double *v)
{
        oquam_stepper_controller_t *c = (oquam_stepper_controller_t*) controller;
        r_err("oquam_stepper_controller_moveat: not implemented");
        return -1;
}
