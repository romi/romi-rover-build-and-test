#include <r.h>
#include "cnc.h"
#include "v.h"
#include "stepper.h"
#include "planner.h"
#include "plotter.h"
#include "ostepper.h"

/**************************************************************************/

enum {
        CONTROLLER_IDLE,
        CONTROLLER_EXECUTING,
        CONTROLLER_TRIGGER
};

typedef struct _oquam_stepper_controller_t {
        stepper_controller_t stepper;        
        double period;
        membuf_t *buffer;
        char *device;
        serial_t *serial;
        thread_t *thread;

        int status;
        int available;
        int block_id;
        int block_ms;
        int milliseconds;
        int interrupts;
        int trigger;
        int32_t stepper_position[3];
        int32_t encoder_position[3];
        uint32_t millis;
        
} oquam_stepper_controller_t;

static void delete_oquam_stepper_controller(controller_t *controller);
static int oquam_stepper_controller_run(controller_t *controller, cnc_t *cnc,
                                          planner_t *planner, int async);
static int oquam_stepper_controller_position(controller_t *controller, double *p);
static int oquam_stepper_controller_moveat(controller_t *controller, double *v);
static int oquam_stepper_controller_execute(oquam_stepper_controller_t* controller,
                                              block_t *block, int len);
static int oquam_stepper_controller_get_status(oquam_stepper_controller_t* controller);
static void oquam_stepper_controller_update_status(void *ptr);


controller_t *new_oquam_stepper_controller(const char *device,
                                           double *xmax, double *vmax, double *amax,
                                           double *scale, double period)
{
        oquam_stepper_controller_t *controller = r_new(oquam_stepper_controller_t);
        if (controller == NULL)
                return NULL;
        
        controller->period = period;        
        vcopy(controller->stepper.interface.xmax, xmax);
        vcopy(controller->stepper.interface.vmax, vmax);
        vcopy(controller->stepper.interface.amax, amax);
        vcopy(controller->stepper.scale, scale);

        controller->stepper.interface.run = oquam_stepper_controller_run;
        controller->stepper.interface.position = oquam_stepper_controller_position;
        controller->stepper.interface.moveat = oquam_stepper_controller_moveat;
        controller->stepper.interface.del = delete_oquam_stepper_controller;
        controller->stepper.execute = (stepper_controller_execute_blocks_t) oquam_stepper_controller_execute;

        ///
        
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
        
        //controller->serial = new_serial(device, 115200, 1);
        controller->serial = new_serial(device, 38400, 1);
        if (controller->serial == NULL) {
                delete_oquam_stepper_controller((controller_t *)controller);
                return NULL;
        }

        ////

        if (stepper_controller_init(&controller->stepper) != 0) {
                delete_oquam_stepper_controller((controller_t *)controller);
                return NULL;
        }

        controller->thread = new_thread(oquam_stepper_controller_update_status,
                                        controller, 0, 0);
        
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

static void oquam_stepper_controller_update_status(void *ptr)
{
        oquam_stepper_controller_t *controller = (oquam_stepper_controller_t*) ptr;

        while (1) {
                oquam_stepper_controller_get_status(controller);
                clock_sleep(0.050);
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

        if (1) {
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

                FILE *fp = fopen("oquam-stepper.txt", "w");
                if (fp) {
                        for (int i = 0; i < stepper->num_blocks; i++) {
                                switch (stepper->block[i].type) {
                                case BLOCK_WAIT:
                                        fprintf(fp, "W\n");
                                        break;
                                case BLOCK_MOVE:
                                        fprintf(fp, "M[%d,%d,%d,%d,%d]\n",
                                                stepper->block[i].data[0],
                                                stepper->block[i].data[1],
                                                stepper->block[i].data[2],
                                                stepper->block[i].data[3],
                                                stepper->block[i].id);
                                        break;
                                case BLOCK_DELAY:
                                        fprintf(fp, "D%d\n", stepper->block[i].data[0]);
                                        break;
                                case BLOCK_TRIGGER:
                                        fprintf(fp, "T%d\n", stepper->block[i].data[0]);
                                        break;
                                }
                        }
                        fclose(fp);
                }
        }
        
        if (stepper_controller_execute(stepper, async) != 0)
                return -1;
        
        return 0;
}

static int oquam_stepper_controller_get_status(oquam_stepper_controller_t* controller)
{
        membuf_clear(controller->buffer);

        const char *r = serial_command_send(controller->serial, controller->buffer, "S");
        r_debug("S -> %s", r);
        
        if (strncmp(r, "ERR", 3) == 0) {
                r_err("oquam_stepper_controller_get_status: %s", r);
                return -1;
        }

        json_object_t s = json_parse(r+1);
        if (!json_isarray(s)) {
                r_err("oquam_stepper_controller_get_status: expected an array: got %s",
                      r+1);
                return -1;
        }
        
        const char* status = json_array_getstr(s, 0);
        if (status == NULL) {
                r_err("oquam_stepper_controller_get_status: empty status: got %s",
                      r+1);
                return -1;
        } else if (rstreq(status, "e")) {
                controller->status = CONTROLLER_EXECUTING;
        } else if (rstreq(status, "i")) {
                controller->status = CONTROLLER_IDLE;
        } else if (rstreq(status, "t")) {
                controller->status = CONTROLLER_TRIGGER;
        }

        controller->available = (int) json_array_getnum(s, 1);
        controller->block_id = (int) json_array_getnum(s, 2);
        controller->block_ms = (int) json_array_getnum(s, 3);
        controller->milliseconds = (int) json_array_getnum(s, 4);
        controller->interrupts = (int) json_array_getnum(s, 5);
        controller->trigger = (int) json_array_getnum(s, 6);
        controller->stepper_position[0] = (int32_t) json_array_getnum(s, 7);
        controller->stepper_position[1] = (int32_t) json_array_getnum(s, 8);
        controller->stepper_position[2] = (int32_t) json_array_getnum(s, 9);
        controller->encoder_position[0] = (int32_t) json_array_getnum(s, 10);
        controller->encoder_position[1] = (int32_t) json_array_getnum(s, 11);
        controller->encoder_position[2] = (int32_t) json_array_getnum(s, 12);
        controller->millis = (uint32_t) json_array_getnum(s, 13);

        if (controller->available > 0
            || controller->block_id >= 0) {
                r_debug("Block %d, %d blocks avail., "
                        "block dur. %d, block ms. %d, "
                        "pos. [%d,%d,%d]s=[%.3f,%.3f,%.3f]m",
                        controller->block_id,
                        controller->available,
                        controller->block_ms,
                        controller->milliseconds,
                        controller->stepper_position[0],
                        controller->stepper_position[1],
                        controller->stepper_position[2],
                        (double) controller->stepper_position[0] / controller->stepper.scale[0],
                        (double) controller->stepper_position[1] / controller->stepper.scale[1],
                        (double) controller->stepper_position[2] / controller->stepper.scale[2]);
        }
        
        return -1;
}

static int oquam_stepper_controller_get_position(oquam_stepper_controller_t* controller)
{
        membuf_clear(controller->buffer);

        const char *r = serial_command_send(controller->serial, controller->buffer, "P");
        r_debug("P -> %s", r);
        
        if (strncmp(r, "ERR", 3) == 0) {
                r_err("oquam_stepper_controller_get_position: %s", r);
                return -1;
        }

        // TODO
        
        return -1;
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
                rprintf(cmd, sizeof(cmd), "W");
                break;
        case BLOCK_MOVE:
                if (block->data[0] <= 0)
                        return 0;
                if (block->data[1] == 0
                    && block->data[2] == 0
                    && block->data[3] == 0)
                        return 0;
                rprintf(cmd, sizeof(cmd), "M[%d,%d,%d,%d,%d]",
                        block->data[0],
                        block->data[1],
                        block->data[2],
                        block->data[3],
                        block->id);
                break;
        case BLOCK_DELAY:
                if (block->data[0] <= 0)
                        return 0;
                rprintf(cmd, sizeof(cmd), "D%d", block->data[0]);
                break;
        case BLOCK_TRIGGER:
                rprintf(cmd, sizeof(cmd), "T%d", block->data[0]);
                break;
        default:
                r_err("oquam_stepper_controller_send_block: Unknown block type");
                return -1;
        }

        while (1) {
                //int err = 0;
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
        while (controller->available > 0
               || controller->block_id >= 0) {
                clock_sleep(0.200);
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
