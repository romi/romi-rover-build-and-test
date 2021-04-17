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
#include "block.h"
#include "stepper.h"
#include "config.h"
#include <ArduinoSerial.h>
#include <RomiSerial.h>

/**
 *  \brief The possible states of the controller (the main thread).
 */
enum {
        STATE_RUNNING = 'r',
        STATE_PAUSED = 'p',
        STATE_HOMING = 'h',
        STATE_ERROR = 'e'
};

extern volatile int32_t stepper_position[3];
extern volatile int8_t stepper_reset;
extern volatile block_t *current_block;

uint8_t controller_state;

void handle_moveto(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_move(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_moveat(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_pause(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_reset(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_position(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_idle(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_homing(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_set_homing(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_enable(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_spindle(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { 'm', 4, false, handle_moveto },
        { 'M', 4, false, handle_move },
        { 'V', 3, false, handle_moveat },
        { 'p', 0, false, handle_pause },
        { 'c', 0, false, handle_continue },
        { 'r', 0, false, handle_reset },
        { 'P', 0, false, send_position },
        { 'I', 0, false, send_idle },
        { 'H', 0, false, handle_homing },
        { 'h', 3, false, handle_set_homing },
        { 'E', 1, false, handle_enable },
        { 'S', 1, false, handle_spindle },
        { '?', 0, false, send_info },
};

ArduinoSerial serial(Serial);
RomiSerial romiSerial(serial, serial, handlers, sizeof(handlers) / sizeof(MessageHandler));


// DEBUG
extern int32_t accumulation_error[3];

static char reply_string[80];
static int8_t homing_axes[3] =  {-1, -1, -1};
static uint8_t limit_switches[3] = {0, 0, 0};


void reset()
{
        block_buffer_clear();
        stepper_reset = 1;
}

void zero()
{
        for (int i = 0; i < 3; i++) {
                stepper_position[i] = 0;
                encoder_position[i] = 0;
                accumulation_error[i] = 0;
        }
}

/**
 * \brief Check the accuracy of the path following.
 *
 * Check that the difference between the actual position of the arm,
 * as measured by the encoders, and the supposed position of the arm,
 * as measured by the number of executed motor steps, is less than a
 * given threshold. If the deviation is larger than the threshold then
 * the controller will stop the execution and request the controlling
 * program to make the necessary adjustments.
 */
void check_accuracy()
{
}

void setup()
{
        Serial.begin(115200);
        while (!Serial)
                ;
        
        init_block_buffer();
        init_pins();
        
        //enable_driver();

        // FIXME: don't belong here
        for (int i = 0; i < 3; i++) {
                stepper_position[i] = 0;
                accumulation_error[i] = 0;
        }
        
        cli();
        init_stepper_timer();
        init_reset_step_pins_timer();
        sei(); 

        controller_state = STATE_RUNNING;
        enable_stepper_timer();
}

static unsigned long last_time = 0;
static unsigned long last_print_time = 0;
static int16_t id = 0;

void loop()
{
        romiSerial.handle_input();
        check_accuracy();
        delay(1);
}

void handle_moveto(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (controller_state == STATE_RUNNING
            || controller_state == STATE_PAUSED) {
                if (args[0] > 0) {
                        block_t *block = block_buffer_get_empty();
                        if (block == 0) {
                                romiSerial->send_error(1, "Again");  
                        } else {
                                block->type = BLOCK_MOVETO;
                                block->data[DT] = args[0];
                                block->data[DX] = args[1];
                                block->data[DY] = args[2];
                                block->data[DZ] = args[3];
                                block_buffer_ready();
                                romiSerial->send_ok();  
                        }
                } else {
                        romiSerial->send_error(100, "Invalid DT");  
                }
        } else {
                romiSerial->send_error(101, "Invalid state");  
        }
}

void handle_move(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (controller_state == STATE_RUNNING
            || controller_state == STATE_PAUSED) {
                if (args[0] > 0) {
                        block_t *block = block_buffer_get_empty();
                        if (block == 0) {
                                romiSerial->send_error(1, "Again");  
                        } else {
                                block->type = BLOCK_MOVE;
                                block->data[DT] = args[0];
                                block->data[DX] = args[1];
                                block->data[DY] = args[2];
                                block->data[DZ] = args[3];
                                block_buffer_ready();
                                romiSerial->send_ok();  
                        }
                } else {
                        romiSerial->send_error(100, "Invalid DT");                  
                }
        } else {
                romiSerial->send_error(101, "Invalid state");  
        }
}

void handle_moveat(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (controller_state == STATE_RUNNING
            || controller_state == STATE_PAUSED) {
                block_t *block = block_buffer_get_empty();
                if (block == 0) {
                        romiSerial->send_error(1, "Again");  
                } else {
                        block->type = BLOCK_MOVEAT;
                        block->data[DT] = 1000;
                        block->data[DX] = args[0];
                        block->data[DY] = args[1];
                        block->data[DZ] = args[2];
                        block_buffer_ready();
                        romiSerial->send_ok();  
                }
        } else {
                romiSerial->send_error(101, "Invalid state");  
        }
}

void handle_pause(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (controller_state == STATE_RUNNING) {
                disable_stepper_timer();
                controller_state = STATE_PAUSED;
                romiSerial->send_ok();  
        } else if (controller_state == STATE_PAUSED) {
                romiSerial->send_ok();  
        } else {
                romiSerial->send_error(101, "Invalid state");  
        }
}

void handle_continue(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (controller_state == STATE_PAUSED) {
                controller_state = STATE_RUNNING;
                enable_stepper_timer();
                romiSerial->send_ok();  
        } else if (controller_state == STATE_RUNNING) {
                romiSerial->send_ok();  
        } else {
                romiSerial->send_error(101, "Invalid state");  
        }
}

void handle_reset(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (controller_state == STATE_PAUSED) {
                controller_state = STATE_RUNNING;
                reset();
                enable_stepper_timer();
                romiSerial->send_ok();  
        } else if (controller_state == STATE_RUNNING) {
                reset();
                romiSerial->send_ok();  
        } else {
                romiSerial->send_error(101, "Invalid state");  
        }
}

void send_position(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        snprintf(reply_string, sizeof(reply_string),
                 "[0,%ld,%ld,%ld]",
                 (int32_t) stepper_position[0],
                 (int32_t) stepper_position[1],
                 (int32_t) stepper_position[2]);
        
        romiSerial->send(reply_string); 
}

static inline bool is_idle()
{
        return ((controller_state == STATE_RUNNING)
                && (block_buffer_available() == 0)
                && (current_block == 0));
}

static inline void wait()
{
        while (!is_idle()) {
                romiSerial.handle_input();
                delay(1);
        }
}

void send_idle(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        snprintf(reply_string, sizeof(reply_string), "[0,%d,\"%c\"]",
                 is_idle(), controller_state);
        romiSerial->send(reply_string); 
}

int moveat(int dt, int dx, int dy, int dz)
{
        int err = 0;
        block_t *block = block_buffer_get_empty();
        if (block == 0) {
                err = -1;
        } else {                
                block->type = BLOCK_MOVEAT;
                block->data[DT] = dt;
                block->data[DX] = dx;
                block->data[DY] = dy;
                block->data[DZ] = dz;
                block_buffer_ready();
        }
        return err;
}

int move(int dt, int dx, int dy, int dz)
{
        int err = 0;
        block_t *block = block_buffer_get_empty();
        if (block == 0) {
                err = -1;
        } else {
                block->type = BLOCK_MOVE;
                block->data[DT] = dt;
                block->data[DX] = dx;
                block->data[DY] = dy;
                block->data[DZ] = dz;
                block_buffer_ready();
        }
        return err;
}

void update_limit_switches()
{
        limit_switches[0] = digitalRead(PIN_LIMIT_SWITCH_X);
        limit_switches[1] = digitalRead(PIN_LIMIT_SWITCH_Y);
        limit_switches[2] = digitalRead(PIN_LIMIT_SWITCH_Z);
}

int homing_move(int dt, int delta, int axis)
{
        int r;
        if (axis == 0)
                r = move(dt, delta, 0, 0);
        else if (axis == 1)
                r = move(dt, 0, delta, 0);
        else if (axis == 2)
                r = move(dt, 0, 0, delta);
        return r;
}

int homing_moveat(int dt, int v, int axis)
{
        int r;
        if (axis == 0)
                r = moveat(dt, v, 0, 0);
        else if (axis == 1)
                r = moveat(dt, 0, v, 0);
        else if (axis == 2)
                r = moveat(dt, 0, 0, v);
        return r;
}

int homing_wait_switch(int dt, int v, int axis, int state)
{
        int err = 0;
        int dx, dy, dz;

        // Serial.print("#!Axis=");
        // Serial.print(axis);
        // Serial.print(":xxxx\r\n");

        err = homing_moveat(dt, v, axis);
        if (err != 0)
                return err;
        
        while (1) {
                update_limit_switches();
                // Serial.print("#!L=");
                // Serial.print(limit_switches[axis]);
                // Serial.print(":xxxx\r\n");
                
                if (limit_switches[axis] == state) {
                        err = move(0, 0, 0, 0); // This will stop the moveat
                        break;
                }
                
                romiSerial.handle_input();
                delay(1);
        }
        return err;
}

int homing_moveto_switch_pressed(int axis)
{
        return homing_wait_switch(1000, -1000, axis, LOW);
}

int homing_moveto_switch_released(int axis)
{
        return homing_wait_switch(300, 1000, axis, HIGH);
}

bool do_homing_axis(int axis)
{
        bool success = false; 
        if (homing_moveto_switch_pressed(axis) == 0 
            && homing_moveto_switch_released(axis) == 0
            && homing_move(100, 100, axis) == 0) {
                // Don't remove the RUNNING because wait() depends on
                // it!
                controller_state = STATE_RUNNING;
                wait();
                success = true;
        }
        return success;
}

bool do_homing()
{
        bool success = true;
        for (int i = 0; i < 3; i++) {
                if (homing_axes[i] >= 0 && homing_axes[i] < 3) {
                        success = do_homing_axis(homing_axes[i]);
                        if (!success)
                                break;
                }
        }
        return success;
}

void handle_homing(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        // Stop whatever is ongoing
        reset();

        // Make sure the timer is running
        enable_stepper_timer();

        controller_state = STATE_HOMING;

        // Reply the client immediately because there's a timeout on
        // sending a response.
        romiSerial->send_ok();
        
        if (do_homing()) {
                reset();
                zero();
        } else {
                controller_state = STATE_ERROR;
        }
}

void handle_set_homing(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        for (int i = 0; i < 3; i++)
                homing_axes[i] = args[i];        
        romiSerial->send_ok();
}

void handle_enable(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (args[0] == 0) {
                disable_driver();
        } else {
                enable_driver()
        }
        romiSerial->send_ok();
}

void handle_spindle(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (args[0] == 0) {
                digitalWrite(PIN_SPINLDE, LOW);
        } else {
                digitalWrite(PIN_SPINLDE, HIGH);
        }
        romiSerial->send_ok();
}

void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send("[\"Oquam\",\"0.1\"]"); 
}

