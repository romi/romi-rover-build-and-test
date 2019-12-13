#include <r.h>
#include "cnc.h"
#include "v.h"
#include "stepper.h"
#include "planner.h"
#include "plotter.h"

int stepper_controller_init(stepper_controller_t* stepper)
{
        stepper->num_blocks = 0;
        stepper->block_length = 1024;
        stepper->block = r_array(block_t, stepper->block_length);
        if (stepper->block == NULL) {
                stepper_controller_cleanup(stepper);
                return -1;
        }
        stepper->block_id = 0;
        
        stepper->num_triggers = 0;
        stepper->trigger_length = 256;
        stepper->trigger = r_array(trigger_t, stepper->trigger_length);
        if (stepper->trigger == NULL) {
                stepper_controller_cleanup(stepper);
                return -1;
        }

        stepper->mutex = new_mutex();
        if (stepper->mutex == NULL) {
                stepper_controller_cleanup(stepper);
                return -1;
        }

        return 0;
}

void stepper_controller_cleanup(stepper_controller_t* stepper)
{
        if (stepper) {
                stepper->thread_done = 1;
                if (stepper->thread) {
                        thread_join(stepper->thread);
                        delete_thread(stepper->thread);
                }
                if (stepper->mutex) 
                        delete_mutex(stepper->mutex);
                if (stepper->block) 
                        r_free(stepper->block);
                if (stepper->trigger) 
                        r_free(stepper->trigger);
                r_delete(stepper);
        }
}

static int stepper_controller_expand_blocks(stepper_controller_t* stepper)
{
        int len;
        block_t *p;
        
        len = stepper->block_length + 1024;
        p = (block_t *) r_realloc(stepper->block, len * sizeof(block_t));
        if (p == NULL)
                return -1;
        
        stepper->block = p;
        stepper->block_length = len;
        return 0;
}

static int stepper_controller_push_block(stepper_controller_t* stepper,
                                         block_t *block)
{
        if (stepper->num_blocks == stepper->block_length
            && stepper_controller_expand_blocks(stepper) != 0)
                return -1;
        
        block_t *p = &stepper->block[stepper->num_blocks];
        memcpy(p, block, sizeof(block_t));
        stepper->num_blocks++;
        return 0;
}

static int stepper_controller_expand_triggers(stepper_controller_t* stepper)
{
        int len;
        trigger_t *p;
        
        len = stepper->trigger_length + 256;
        p = (trigger_t *) r_realloc(stepper->trigger, len * sizeof(trigger_t));
        if (p == NULL)
                return -1;
        
        stepper->trigger = p;
        stepper->trigger_length = len;
        return 0;
}

static int stepper_controller_push_trigger(stepper_controller_t* stepper,
                                           trigger_t *trigger)
{
        if (stepper->num_triggers == stepper->trigger_length
            && stepper_controller_expand_triggers(stepper) != 0)
                return -1;
        
        trigger_t *p = &stepper->trigger[stepper->num_triggers];
        memcpy(p, trigger, sizeof(trigger_t));
        stepper->num_triggers++;
        return 0;
}

static int stepper_controller_push_wait(stepper_controller_t* stepper)
{
        block_t block;
        block.type = BLOCK_WAIT;
        return stepper_controller_push_block(stepper, &block);
}

static int stepper_controller_push_move(stepper_controller_t* stepper,
                                        section_t *section, int32_t *pos_steps)
{
        block_t block;
        block.type = BLOCK_MOVE;

        int32_t p1[3];
        p1[0] = (int32_t) (section->p1[0] * stepper->scale[0]);
        p1[1] = (int32_t) (section->p1[1] * stepper->scale[1]);
        p1[2] = (int32_t) (section->p1[2] * stepper->scale[2]);

        block.data[0] = (int16_t) (1000.0 * section->t);
        block.data[1] = (int16_t) (p1[0] - pos_steps[0]);
        block.data[2] = (int16_t) (p1[1] - pos_steps[1]);
        block.data[3] = (int16_t) (p1[2] - pos_steps[2]);

        block.id = stepper->block_id++;
        
        // Keep the IDs reasonbly low so that the messages don't
        // become too long.
        if (stepper->block_id == 10000) 
                stepper->block_id = 0;
                
        pos_steps[0] = p1[0];
        pos_steps[1] = p1[1];
        pos_steps[2] = p1[2];
        
        return stepper_controller_push_block(stepper, &block);
}

static int stepper_controller_push_delay(stepper_controller_t* stepper,
                                         action_t *action)
{
        block_t block;
        uint32_t milliseconds;

        milliseconds = (uint32_t) (action->delay * 1000.0f);        
        block.type = BLOCK_DELAY;
        
        while (milliseconds > 0) {
                if (milliseconds > 32767) {
                        block.data[0] = 32767;
                        milliseconds -= 32767;
                } else {
                        block.data[0] = milliseconds;
                        milliseconds = 0;
                }
                if (stepper_controller_push_block(stepper, &block) != 0)
                        return -1;
        }
        return 0;
}

static int stepper_controller_push_trigger_action(stepper_controller_t* stepper,
                                                  action_t *action, cnc_t *cnc)
{
        block_t block;
        trigger_t trigger;

        trigger.id = stepper->num_triggers;
        trigger.arg = action->arg;
        trigger.callback = action->callback;
        trigger.userdata = action->userdata;
        trigger.cnc = cnc;
        
        block.type = BLOCK_TRIGGER;
        block.data[0] = trigger.id;
        
        if (stepper_controller_push_trigger(stepper, &trigger) != 0)
                return -1;

        if (stepper_controller_push_block(stepper, &block) != 0)
                return -1;
        
        return 0;
}

static int stepper_controller_push_action(stepper_controller_t* stepper,
                                          action_t *action, cnc_t *cnc)
{
        int err = 0;
        
        switch (action->type) {
        case ACTION_WAIT:
                err = stepper_controller_push_wait(stepper);
                break;
        case ACTION_MOVE:
                r_err("Found move action while compiling");
                err = -1;
                break;
        case ACTION_DELAY:
                err = stepper_controller_push_delay(stepper, action);
                break;
        case ACTION_TRIGGER:
                err = stepper_controller_push_trigger_action(stepper, action, cnc);
                break;
        }
        
        return err;
}

static int stepper_controller_push_actions(stepper_controller_t* stepper,
                                           section_t *section, cnc_t *cnc)
{
        for (list_t *l = section->actions; l != NULL; l = list_next(l)) {
                action_t *action = list_get(l, action_t);
                if (stepper_controller_push_action(stepper, action, cnc) != 0)
                        return -1;
        }
        return 0;
}

int stepper_controller_compile(stepper_controller_t* stepper,
                               list_t *slices, cnc_t *cnc,
                               planner_t *planner)
{
        int err = -1;
        
        mutex_lock(stepper->mutex);
        
        int32_t pos_steps[3];
        section_t *section = list_get(slices, section_t);

        if (section) {
                pos_steps[0] = (int32_t) (section->p0[0] * stepper->scale[0]);
                pos_steps[1] = (int32_t) (section->p0[1] * stepper->scale[1]);
                pos_steps[2] = (int32_t) (section->p0[2] * stepper->scale[2]);
        }
        
        for (list_t *l = slices; l != NULL; l = list_next(l)) {
                section = list_get(l, section_t);

                if (section->t > 0) {
                        if (stepper_controller_push_move(stepper, section, pos_steps) != 0)
                                goto unlock_and_return;
                        
                } else if (section->actions) {
                        if (stepper_controller_push_actions(stepper, section, cnc) != 0)
                                goto unlock_and_return;
                }
        }

        err = 0;
        
unlock_and_return:
        
        mutex_unlock(stepper->mutex);
        return err;
}

static int stepper_controller_run(stepper_controller_t* stepper)
{
        int err = stepper->execute(stepper, stepper->block, stepper->num_blocks);
        
        if (stepper->thread) {
                delete_thread(stepper->thread);
                stepper->thread = NULL;
        }
        
        return err;
}

int stepper_controller_execute(stepper_controller_t* stepper, int async)
{
        if (stepper->execute == NULL) {
                r_warn("No execute function is defined");
                return -1;
        }
        
        if (stepper->thread) {
                r_warn("A thread is already executing");
                return -1;
        }
        
        if (async) {
                stepper->thread_done = 0;
                stepper->thread = new_thread((thread_run_t) stepper_controller_run,
                                             stepper, 0, 0);
                return 0;
                
        } else {
                return stepper_controller_run(stepper);
        }
}

