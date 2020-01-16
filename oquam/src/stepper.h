#include "controller.h"

#ifndef _OQUAM_STEPPER_H_
#define _OQUAM_STEPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
        BLOCK_WAIT = 0,
        BLOCK_MOVE,
        BLOCK_MOVEAT,
        BLOCK_DELAY,
        BLOCK_TRIGGER
};

typedef struct _block_t {
        uint8_t type;
        int16_t id;
        int16_t data[4];
} block_t;


typedef struct _trigger_t {
        int16_t id;
        int arg;
        cnc_callback_t callback;
        void *userdata;
        cnc_t *cnc;
} trigger_t;


typedef struct _stepper_controller_t stepper_controller_t;


typedef int (*stepper_controller_execute_blocks_t)(stepper_controller_t *stepper,
                                                   block_t *block,
                                                   int length);

struct _stepper_controller_t {

        controller_t interface;

        block_t *block;
        int32_t num_blocks;
        int32_t block_length;
        int16_t block_id;
        
        trigger_t *trigger;
        int32_t num_triggers;
        int32_t trigger_length;

        thread_t *thread;
        mutex_t *mutex;
        int thread_done;

        /** 
         * The scale converts a distance in meters to the
         * corresponding number of steps, steps = meters x scale, 
         * or meters = steps/scale.
         */
        double scale[3];
        
        stepper_controller_execute_blocks_t execute;
        
};

int stepper_controller_init(stepper_controller_t* stepper);
void stepper_controller_cleanup(stepper_controller_t* stepper);

int stepper_controller_compile(stepper_controller_t* stepper,
                               list_t *slices, cnc_t *cnc,
                               planner_t *planner);
int stepper_controller_execute(stepper_controller_t* stepper, int async);

void stepper_controller_do_trigger(stepper_controller_t* stepper, int id);

#ifdef __cplusplus
}
#endif

#endif // _OQUAM_STEPPER_H_
