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
#include "script.h"

action_t *new_action(int type)
{
        action_t *action = r_new(action_t);
        if (action == NULL)
                return NULL;
        action->type = type;
        return action;
}

action_t *action_clone(action_t *a)
{
        action_t *action = r_new(action_t);
        if (action == NULL)
                return NULL;
        memcpy(action, a, sizeof(action_t));
        return action;
}

action_t *new_wait()
{
        action_t *action = new_action(ACTION_WAIT);
        if (action == NULL)
                return NULL;
        return action;
}

action_t *new_delay(double delay)
{
        action_t *action = new_action(ACTION_DELAY);
        if (action == NULL)
                return NULL;
        action->data.delay.duration = delay;
        return action;
}

action_t *new_move(double x, double y, double z, double v, int id)
{
        action_t *action = new_action(ACTION_MOVE);
        if (action == NULL)
                return NULL;
        action->data.move.p[0] = x;
        action->data.move.p[1] = y;
        action->data.move.p[2] = z;
        action->data.move.v = v;
        action->data.move.id = id;
        return action;
}

action_t *new_trigger(trigger_callback_t callback, void *userdata, int16_t arg, double delay)
{
        action_t *action = new_action(ACTION_TRIGGER);
        if (action == NULL)
                return NULL;
        action->data.trigger.callback = callback;
        action->data.trigger.userdata = userdata;
        action->data.trigger.arg = arg;
        action->data.trigger.delay = delay;
        return action;
}

void delete_action(action_t *action)
{
        if (action)
                r_delete(action);
}

/*********************************************************************/

script_t *new_script()
{
        script_t *script = r_new(script_t);
        if (script == NULL)
                return NULL;
        return script;
}

void delete_script(script_t *script)
{
        if (script) {
                
                script_clear(script);
                        
                for (list_t *l = script->actions; l; l = list_next(l)) {
                        action_t *n = list_get(l, action_t);
                        delete_action(n);
                }
                delete_list(script->actions);
                
                r_delete(script);
        }
}

void script_clear(script_t *script)
{
        if (script->segments) {
                for (list_t *l = script->segments; l != NULL; l = list_next(l)) {
                        segment_t *s = list_get(l, segment_t);
                        while (s != NULL) {
                                segment_t *next = s->next;
                                delete_segment(s);
                                s = next;
                        }
                }
                delete_list(script->segments);
                script->segments = NULL;
        }

        if (script->atdc_list) {
                for (list_t *l = script->atdc_list; l != NULL; l = list_next(l)) {
                        atdc_t *atdc = list_get(l, atdc_t);
                        while (atdc != NULL) {
                                atdc_t *next = atdc->next;
                                delete_atdc(atdc);
                                atdc = next;
                        }
                }
                delete_list(script->atdc_list);
                script->atdc_list = NULL;
        }

        if (script->slices) {
                for (list_t *l = script->slices; l; l = list_next(l)) {
                        section_t *section = list_get(l, section_t);
                        delete_section(section);
                }
                delete_list(script->slices);
                script->slices = NULL;
        }
        
        if (script->block)
                r_delete(script->block);
        script->block = NULL;
        script->num_blocks = 0;
        script->block_length = 0;
        script->block_id = 0;
        
        if (script->trigger)
                r_delete(script->trigger);
        script->trigger = NULL;
        script->num_triggers = 0;
        script->trigger_length = 0;        
}

static void script_append(script_t *script, action_t *action)
{
        script->actions = list_append(script->actions, action);
}

int script_moveto(script_t *script, double x, double y, double z, double v, int id)
{
        if (v <= 0.0f) {
                r_warn("script_moveto: speed must be positive");
                return -1;
        }
                
        action_t *action = new_move(x, y, z, v, id);
        if (action == NULL)
                return -1;
        
        script_append(script, action);
        return 0;
}

int script_delay(script_t *script, double seconds)
{        
        action_t *action = new_delay(seconds);
        if (action == NULL)
                return -1;
        
        script_append(script, action);
        return 0;
}

int script_trigger(script_t *script, trigger_callback_t cb, void *userdata, int16_t arg, double delay)
{        
        action_t *action = new_trigger(cb, userdata, arg, delay);
        if (action == NULL)
                return -1;
        
        script_append(script, action);
        return 0;
}

static trigger_t *script_get_trigger(script_t* script, int id)
{
        return &script->trigger[id];
}

void script_do_trigger(script_t* script, int id)
{
        trigger_t *trigger = script_get_trigger(script, id);
        trigger->callback(trigger->userdata, trigger->arg);
}

static int script_expand_blocks(script_t* script)
{
        int len;
        block_t *p;
        
        len = script->block_length + 1024;
        p = (block_t *) r_realloc(script->block, len * sizeof(block_t));
        if (p == NULL)
                return -1;
        
        script->block = p;
        script->block_length = len;
        return 0;
}

int script_push_block(script_t* script, block_t *block)
{
        if (script->num_blocks == script->block_length
            && script_expand_blocks(script) != 0)
                return -1;
        
        block_t *p = &script->block[script->num_blocks];
        memcpy(p, block, sizeof(block_t));
        script->num_blocks++;
        return 0;
}

static int script_expand_triggers(script_t* script)
{
        int len;
        trigger_t *p;
        
        len = script->trigger_length + 256;
        p = (trigger_t *) r_realloc(script->trigger, len * sizeof(trigger_t));
        if (p == NULL)
                return -1;
        
        script->trigger = p;
        script->trigger_length = len;
        return 0;
}

int script_push_trigger(script_t* script, trigger_t *trigger)
{
        if (script->num_triggers == script->trigger_length
            && script_expand_triggers(script) != 0)
                return -1;
        
        trigger_t *p = &script->trigger[script->num_triggers];
        memcpy(p, trigger, sizeof(trigger_t));
        script->num_triggers++;
        return 0;
}
