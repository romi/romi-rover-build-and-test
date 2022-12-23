/*
  Romi motor controller for brushed motors

  Copyright (C) 2021 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  MotorController is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

#include <stdio.h>
#include "MotorControllerCommands.h"
#include "MotorControllerVersion.h"

// FIXME
#if ARDUINO
#include "MotorControllerTests.h"
#endif

using namespace romiserial;

const static MessageHandler handlers[] = {
        { '?', 0, false, send_info },
        { 'e', 0, false, send_encoders },
        { 'C', 10, false, handle_configure },
        { 'E', 1, false, handle_enable },
        { 'V', 2, false, handle_moveat },
        { 'X', 0, false, handle_stop },
        { 'v', 0, false, send_speeds },
#if ARDUINO
        { 'T', 1, false, handle_tests },
#endif
        // { 'p', 1, false, send_pid },
        // { 'S', 0, false, send_status },
        // { 'c', 0, false, send_configuration },
};

static IMotorController *controller_ = nullptr;
static IRomiSerial *romi_serial_ = nullptr;
static char reply_buffer[100];

const char *kErrBadState = "Bad state";

void setup_commands(IMotorController *controller, IRomiSerial *romiSerial)
{
        controller_ = controller;
        romi_serial_ = romiSerial;
        if (romi_serial_)
                romi_serial_->set_handlers(handlers, sizeof(handlers) / sizeof(MessageHandler));
}

void handle_commands()
{
        if (controller_ != nullptr
            && romi_serial_ != nullptr)
                romi_serial_->handle_input();
}

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        (void) args;
        (void) string_arg;
        
        romiSerial->send("[0,\"MotorController\",\"" kMotorControllerVersion "\","
                         "\"" __DATE__ " " __TIME__ "\"]"); 
}

void send_encoders(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        (void) args;
        (void) string_arg;
        int32_t left;
        int32_t right;
        uint32_t time;
        controller_->get_encoders(left, right, time);

#if ARDUINO
        snprintf(reply_buffer, sizeof(reply_buffer),
                 "[0,%ld,%ld,%lu]", left, right, time);
#else
        snprintf(reply_buffer, sizeof(reply_buffer),
                 "[0,%d,%d,%u]", left, right, time);
#endif

        romiSerial->send(reply_buffer);                
}

void handle_configure(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        (void) string_arg;
        bool success;
        MotorControllerConfiguration config;

        // Encoders
        config.encoder_steps = (uint16_t) args[0];
        config.left_direction = args[1] > 0? 1 : -1;
        config.right_direction = args[2] > 0? 1 : -1;

        // Envelope
        config.max_speed = (double) args[3] / 1000.0f;
        config.max_acceleration = (double) args[4] / 1000.0f;

        // PI controller
        config.kp_numerator = args[5];
        config.kp_denominator = args[6];
        config.ki_numerator = args[7];
        config.ki_denominator = args[8];
        config.max_amplitude = args[9];
                
        success = controller_->configure(config);
        if (success) {
                romiSerial->send_ok();  
        } else {
                romiSerial->send_error(1, kErrBadState);  
        }
}

void handle_enable(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        (void) string_arg;
        bool success;
        if (args[0] == 0) {
                success = controller_->disable();
        } else {
                success = controller_->enable();
        }
        if (success) {
                romiSerial->send_ok();  
        } else {
                romiSerial->send_error(1, kErrBadState);
        }
}

void handle_moveat(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        (void) string_arg;
        int16_t left = (int16_t) args[0];
        int16_t right = (int16_t) args[1];
        bool success = controller_->set_target_speeds(left, right);
        if (success) {
                romiSerial->send_ok();  
        } else {
                romiSerial->send_error(1, kErrBadState);
        }
}

void handle_stop(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        (void) args;
        (void) string_arg;
        controller_->stop();
        romiSerial->send_ok();  
}

void send_speeds(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        (void) string_arg;
        (void) args;
        int16_t left_target;
        int16_t right_target;
        int16_t left_current;
        int16_t right_current;
        int16_t left_measured;
        int16_t right_measured;

        controller_->get_target_speeds(left_target, right_target);
        controller_->get_current_speeds(left_current, right_current);
        controller_->get_measured_speeds(left_measured, right_measured);
        
        snprintf(reply_buffer, sizeof(reply_buffer),
                 "[0,%hd,%hd,%hd,%hd,%hd,%hd]",
                 left_target, right_target,
                 left_current, right_current,
                 left_measured, right_measured);
        romiSerial->send(reply_buffer);                
}

#if ARDUINO
void handle_tests(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        (void) string_arg;
        romiSerial->send_ok();  
        run_tests(*controller_, args[0]);
}
#endif
