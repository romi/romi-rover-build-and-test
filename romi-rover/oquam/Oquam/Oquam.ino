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
#include "limit.h"
#include "encoder.h"
#include <RomiSerial.h>

/**
 *  \brief The possible states of the controller (the main thread).
 */
enum {
        STATE_RUNNING,
        STATE_HOMING,
        STATE_ERROR
};

extern volatile uint8_t thread_state;
extern volatile int32_t interrupts;
extern volatile int32_t counter_reset_timer;
extern volatile int32_t stepper_position[3];
extern volatile int16_t milliseconds;
extern volatile int16_t block_id;
extern volatile int16_t block_ms;
extern volatile int8_t stepper_reset;

uint8_t controller_state = STATE_RUNNING;

void handle_moveto(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_move(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_moveat(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_continue(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_state(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_homing(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { 'm', 5, false, handle_moveto },
        { 'M', 5, false, handle_move },
        { 'V', 4, false, handle_moveat },
        { 'C', 0, false, handle_continue },
        { 'S', 0, false, send_state },
        { 'H', 0, false, handle_homing },
        { '?', 0, false, send_info },
};

RomiSerial romiSerial(handlers, sizeof(handlers) / sizeof(MessageHandler));


// DEBUG
extern int32_t accumulation_error[3];

static char reply_string[80];

void reset()
{
        stepper_reset = 1;
        block_buffer_clear();
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

        controller_state = STATE_RUNNING;
        
        init_block_buffer();
        init_output_pins();
        init_encoders();
        
        enable_driver();

        // FIXME: don't belong here
        for (int i = 0; i < 3; i++) {
                stepper_position[i] = 0;
                accumulation_error[i] = 0;
        }
        
        cli();
        init_stepper_timer();
        init_reset_step_pins_timer();
        sei(); 

        enable_stepper_timer();
}

static unsigned long last_time = 0;
static unsigned long last_print_time = 0;
static int16_t id = 0;

void loop()
{
        romiSerial.handle_input();
        check_accuracy();
}

void handle_moveto(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (controller_state == STATE_RUNNING) {
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
                                block->id = args[4];
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
        if (controller_state == STATE_RUNNING) {
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
                                block->id = args[4];
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
        if (controller_state == STATE_RUNNING) {
                block_t *block = block_buffer_get_empty();
                if (block == 0) {
                        romiSerial->send_error(1, "Again");  
                } else {
                        block->type = BLOCK_MOVEAT;
                        block->data[DT] = 1000;
                        block->data[DX] = args[0];
                        block->data[DY] = args[1];
                        block->data[DZ] = args[2];
                        block->id = args[3];
                        block_buffer_ready();
                        romiSerial->send_ok();  
                }
        } else {
                romiSerial->send_error(101, "Invalid state");  
        }
}

void handle_continue(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (controller_state == STATE_RUNNING) {
                enable_stepper_timer();
                romiSerial->send_ok();  
        } else {
                romiSerial->send_error(101, "Invalid state");  
        }
}

void send_state(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        uint8_t n = block_buffer_available();
        char *status;
        
        switch (controller_state) {
        case STATE_RUNNING:
                status = "r";
                break;
        case STATE_HOMING:
                status = "";
                break;
        case STATE_ERROR:
                status = "e";
                break;
        }
        
        snprintf(reply_string, sizeof(reply_string),
                 "[0,\"%s\",%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld,%ld,%ld,%lu]",
                 status,
                 (int) block_buffer_available(),
                 (int) block_id,
                 (int) block_ms,
                 (int) milliseconds,
                 (int) interrupts,
                 (int32_t) stepper_position[0],
                 (int32_t) stepper_position[1],
                 (int32_t) stepper_position[2],
                 (int32_t) encoder_position[0],
                 (int32_t) encoder_position[1],
                 (int32_t) encoder_position[2],
                 millis());
        
        romiSerial->send(reply_string); 
}

void wait()
{
        while (block_buffer_available() > 0
               || block_id != -1) {
                delay(10);
        }
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
                block->id = 0;
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
                block->id = 0;
                block_buffer_ready();
        }
        return err;
}

int homing_wait_xy_switches_toggle(int dt, int dx, int dy, int state)
{
        int err = 0;
        
        err = moveat(dt, dx, dy, 0);
        if (err != 0) return err;
        
        while (1) {
                int x_limit_status = digitalRead(9);
                int y_limit_status = digitalRead(10);
                
                if (x_limit_status == state && y_limit_status == state) {
                        err = move(0, 0, 0, 0); // This will stop the moveat
                        break;
                        
                } else if (x_limit_status == state) {
                        err = moveat(dt, 0, dy, 0);
                        if (err != 0) break;
                        
                } else if (y_limit_status == state) {
                        err = moveat(dt, dx, 0, 0); 
                        if (err != 0) break;
               }
                        
                delay(1);
        }
        return err;
}

int homing_moveto_xy_switches_pressed()
{
        return homing_wait_xy_switches_toggle(1000, -1000, -1000, LOW);
}

int homing_moveto_xy_switches_released()
{
        return homing_wait_xy_switches_toggle(300, 1000, 1000, HIGH);
}

bool do_homing()
{
        bool success = false; 
        if (homing_moveto_xy_switches_pressed() == 0 
            && homing_moveto_xy_switches_released() == 0
            && move(100, 100, 100, 0) == 0) {
                wait();
                success = true;
        }
        return success;
}

void handle_homing(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        controller_state = STATE_HOMING;
        romiSerial->send_ok();
        
        if (do_homing()) {
                reset();
                zero();
                controller_state = STATE_RUNNING;
        } else {
                controller_state = STATE_ERROR;
        }
}

void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send("[\"Oquam\",\"0.1\"]"); 
}

