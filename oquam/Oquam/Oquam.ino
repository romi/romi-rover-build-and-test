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
#include "Parser.h"

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
Parser parser("mMDTV", "?SCWXZ");

// DEBUG
extern int32_t accumulation_error[3];

static char reply_string[80];

#define set_reply(__s)  snprintf(reply_string, sizeof(reply_string), "%s", __s)


char* get_state_string()
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
                 "S['%s',%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%ld,%ld,%ld,%lu]",
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
        return reply_string;
}

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
 * \brief Handle incoming serial data.
 *
 * Only one byte is handled at a time to avoid delaying the other
 * functions.
 */
void handle_input()
{
        if (Serial.available() > 0) {
                char c = Serial.read();
                if (parser.process(c)) {                        
                        switch (parser.opcode()) {
                        default:
                                set_reply("ERR Unknown command");
                                break;
                                
                                /* First handle the commands 'm', 'M',
                                 * 'V', 'D, 'T', 'W' to create a new
                                 * block.
                                 */
                                
                        case 'm':
                                if (parser.length() >= 4
                                    && parser.value(0) > 0) {
                                        block_t *block = block_buffer_get_empty();
                                        if (block == 0) {
                                                set_reply("RE m");
                                        } else {
                                                block->type = BLOCK_MOVETO;
                                                block->data[DT] = parser.value(0);
                                                block->data[DX] = parser.value(1);
                                                block->data[DY] = parser.value(2);
                                                block->data[DZ] = parser.value(3);
                                                if (parser.length() == 5)
                                                        block->id = parser.value(4);
                                                block_buffer_ready();

                                                snprintf(reply_string, sizeof(reply_string),
                                                         "OK m");
                                                
                                        }
                                } else if (parser.value(0) <= 0) {
                                        set_reply("ERR invalid speed");
                                } else if (parser.length() < 4) {
                                        set_reply("ERR missing args");
                                }
                                break;
                        case 'M':
                                if (parser.length() >= 4
                                    && parser.value(0) > 0) {
                                        block_t *block = block_buffer_get_empty();
                                        if (block == 0) {
                                                set_reply("RE M");
                                        } else {
                                                block->type = BLOCK_MOVE;
                                                block->data[DT] = parser.value(0);
                                                block->data[DX] = parser.value(1);
                                                block->data[DY] = parser.value(2);
                                                block->data[DZ] = parser.value(3);
                                                if (parser.length() == 5)
                                                        block->id = parser.value(4);
                                                block_buffer_ready();

                                                snprintf(reply_string, sizeof(reply_string),
                                                         "OK M");
                                        }
                                } else if (parser.value(0) <= 0) {
                                        // Zero duration: just skip
                                        set_reply("OK M");
                                } else if (parser.length() < 4) {
                                        set_reply("ERR missing args");
                                }
                                break;
                        case 'V':
                                if (parser.length() >= 3) {
                                        block_t *block = block_buffer_get_empty();
                                        if (block == 0) {
                                                set_reply("RE V");
                                        } else {
                                                block->type = BLOCK_MOVEAT;
                                                block->data[DT] = 1000;
                                                block->data[DX] = parser.value(0);
                                                block->data[DY] = parser.value(1);
                                                block->data[DZ] = parser.value(2);
                                                if (parser.length() == 4)
                                                        block->id = parser.value(3);
                                                block_buffer_ready();
                                                set_reply("OK V");
                                        }
                                } else if (parser.length() < 3) {
                                        set_reply("ERR missing args");
                                }
                                break;
                        case 'D':
                                if (parser.length() == 1
                                    && parser.value(0) > 0) {
                                        block_t *block = block_buffer_get_empty();
                                        if (block == 0) {
                                                set_reply("RE D");
                                        } else {
                                                block->type = BLOCK_DELAY;
                                                block->data[0] = parser.value(0);
                                                block_buffer_ready();
                                                set_reply("OK D");
                                        }
                                } else {
                                        set_reply("ERR bad arg");
                                }
                                break;
                        case 'T':
                                if (parser.length() == 1
                                    || parser.length() == 2) {
                                        int16_t id, delay = 0;
                                        id = parser.value(2);
                                        if (parser.length() == 2)
                                                delay = parser.value(1);
                                        block_t *block = block_buffer_get_empty();
                                        if (block == 0) {
                                                set_reply("RE T");
                                        } else {
                                                block->type = BLOCK_TRIGGER;
                                                block->data[0] = id;
                                                block->data[1] = delay;
                                                block_buffer_ready();
                                                set_reply("OK T");
                                        }
                                } else {
                                        set_reply("ERR bad arg");
                                }
                                break;
                        case 'W':
                                if (1) {
                                        block_t *block = block_buffer_get_empty();
                                        if (block == 0) {
                                                set_reply("RE W");
                                        } else {
                                                block->type = BLOCK_WAIT;
                                                block_buffer_ready();
                                                set_reply("OK W");
                                        }
                                } else {
                                        set_reply("ERR bad arg");
                                }
                                break;

                                /* The other, 'real-time', commands. */
                                
                        case 'C':
                                enable_stepper_timer();
                                set_reply("OK Cont");
                                break;
                        case 'S':
                                get_state_string();
                                break;
                        case 'X':
                                reset();
                                set_reply("OK Clear");
                                break;
                        case 'Z':
                                reset();
                                zero();
                                set_reply("OK Zero");
                                break;
                        case '?':
                                set_reply("?[\"Oquam\",\"0.1\"]"); 
                                break;
                        }
                        
                        Serial.println(reply_string);
                }
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
        handle_input();
        check_accuracy();

        if (0) {
                unsigned long time = millis();
                if (time - last_time > 100) {
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

