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
#include "trigger.h"
#include "stepper.h"
#include "config.h"
#include "limit.h"
#include "encoder.h"
#include <RomiSerial.h>

/**
 *  \brief The possible states of the controller (the main thread).
 */
enum {
        STATE_INTERACTIVE
};

extern volatile uint8_t thread_state;
extern volatile int32_t interrupts;
extern volatile int32_t counter_reset_timer;
extern volatile int32_t stepper_position[3];
extern volatile int16_t milliseconds;
extern volatile int16_t block_id;
extern volatile int16_t block_ms;
extern volatile int8_t stepper_reset;

uint8_t controller_state;
uint8_t error_number;

void handle_moveto(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_move(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_moveat(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_delay(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_trigger(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_wait(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_continue(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_state(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_reset(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_zero(RomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { 'm', 5, false, handle_moveto },
        { 'M', 5, false, handle_move },
        { 'V', 4, false, handle_moveat },
        { 'D', 1, false, handle_delay },
        { 'T', 2, false, handle_trigger },
        { 'W', 0, false, handle_wait },
        { 'C', 0, false, handle_continue },
        { 'S', 0, false, send_state },
        { 'X', 0, false, handle_reset },
        { 'Z', 0, false, handle_zero },
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
        trigger_buffer_clear();
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
        //Serial.begin(115200);
        Serial.begin(38400);
        while (!Serial)
                ;

        controller_state = STATE_INTERACTIVE;
        
        init_block_buffer();
        init_trigger_buffer();
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

        if (0) {
                unsigned long time = millis();
                if (time - last_time > 400) {
                        block_t *block = block_buffer_get_empty();
                        if (block == 0) {
                                Serial.print("RE ");
                                Serial.println(block_buffer_available());
                        } else {
                                block->type = BLOCK_MOVE;
                                block->id = id++;
                                block->data[DT] = 500;
                                block->data[DX] = 1000;
                                block->data[DY] = 1000;
                                block->data[DZ] = 0;
                                block_buffer_ready();
                                Serial.print("OK ");
                                Serial.println(block_buffer_available());
                        }
                        last_time = time;
                }
        }

        if (0) {
                unsigned long time = millis();
                if (block_id >= 0 && time - last_print_time > 50) {
                        Serial.print(block_id);
                        Serial.print(":");
                        Serial.print(milliseconds);
                        Serial.print(":");
                        Serial.print(interrupts);
                        Serial.print(":");
                        Serial.print(stepper_position[0]);
                        Serial.print(",");
                        Serial.print(stepper_position[1]);
                        Serial.print(",");
                        Serial.print(stepper_position[2]);
                        Serial.println();
                        last_print_time = time;
                }
        }

        if (0) {
                unsigned long time = millis();
                if (time - last_time > 100) {
                        last_time = time;
                        if (reached_x_limit())
                                Serial.println("x");
                        if (reached_y_limit())
                                Serial.println("y");
                        // if (reached_z_limit())
                        //         Serial.println("z");
                }
        }
}

void handle_moveto(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
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
                romiSerial->send_error(100, "Invalid speed");  
        } 
}

void handle_move(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
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
                romiSerial->send_error(100, "Invalid speed");                  
        }
}

void handle_moveat(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
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
}

void handle_delay(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (args[0] > 0) {
                block_t *block = block_buffer_get_empty();
                if (block == 0) {
                        romiSerial->send_error(1, "Again");  
                } else {
                        block->type = BLOCK_DELAY;
                        block->data[0] = args[0];
                        block_buffer_ready();
                        romiSerial->send_ok();  
                }
        } else {
                romiSerial->send_error(100, "Invalid delay");                  
        }
}

void handle_trigger(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (args[1] >= 0) {
                block_t *block = block_buffer_get_empty();
                if (block == 0) {
                        romiSerial->send_error(1, "Again");  
                } else {
                        block->type = BLOCK_TRIGGER;
                        block->data[0] = args[0];
                        block->data[1] = args[1];
                        block_buffer_ready();
                        romiSerial->send_ok();  
                }
        } else {
                romiSerial->send_error(100, "Invalid delay");                  
        }
}

void handle_wait(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        block_t *block = block_buffer_get_empty();
        if (block == 0) {
                romiSerial->send_error(1, "Again");  
        } else {
                block->type = BLOCK_WAIT;
                block_buffer_ready();
                romiSerial->send_ok();  
        }
}

void handle_continue(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        enable_stepper_timer();
        romiSerial->send_ok();  
}

void send_state(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        uint8_t n = block_buffer_available();
        char *status;
        int16_t trigger = -1;
        
        if (trigger_buffer_available() > 0) {
                status = "t";
                trigger = trigger_buffer_get();
        } else {
                switch (thread_state) {
                case STATE_THREAD_EXECUTING:
                        status = "e";
                        break;
                case STATE_THREAD_IDLE:
                        status = "i";
                        break;
                }
        }
        
        snprintf(reply_string, sizeof(reply_string),
                 "[0,'%s',%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld,%ld,%ld,%lu]",
                 status,
                 (int) block_buffer_available(),
                 (int) block_id,
                 (int) block_ms,
                 (int) milliseconds,
                 (int) interrupts,
                 (int) trigger,
                 (int32_t) stepper_position[0],
                 (int32_t) stepper_position[1],
                 (int32_t) stepper_position[2],
                 (int32_t) encoder_position[0],
                 (int32_t) encoder_position[1],
                 (int32_t) encoder_position[2],
                 millis());
        
        romiSerial->send(reply_string); 
}

void handle_reset(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        reset();
        romiSerial->send_ok();  
}

void handle_zero(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        reset();
        zero();
        romiSerial->send_ok();  
}

void send_info(RomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send("[\"Oquam\",\"0.1\"]"); 
}

