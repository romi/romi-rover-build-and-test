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
#include "stepper.h"
#include "config.h"
#include "ArduinoUno.h"
#include <ArduinoSerial.h>
#include <RomiSerial.h>

using namespace romiserial;
/**
 *  \brief The possible states of the controller (the main thread).
 */
enum {
        STATE_RUNNING = 'r',
        STATE_PAUSED = 'p',
        STATE_HOMING = 'h',
        STATE_ERROR = 'e'
};

extern volatile ControlMode mode_;

uint8_t controller_state;
//static int32_t stepper_steps_per_revolution = 30706;
//static int32_t stepper_steps_per_revolution = 245650;
//static int32_t stepper_steps_per_revolution = 61412;
static int32_t stepper_steps_per_revolution = 57000;

static int32_t encoder_steps_per_revolution = (int32_t) 1024;

extern volatile int16_t left_encoder_value_;
extern volatile int16_t right_encoder_value_;


static const char *kInvalidState = "Invalid state";

void handle_moveto(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_pause(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_continue(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_homing(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_enable(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_print(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void change_mode(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void send_idle(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { 'm', 2, false, handle_moveto },
        { 'p', 0, false, handle_pause },
        { 'c', 0, false, handle_continue },
        { 'P', 0, false, send_position },
        { 'H', 0, false, handle_homing },
        { 'E', 1, false, handle_enable },
        { 'D', 1, false, handle_print },
        { 'C', 1, false, change_mode },
        { 'I', 0, false, send_idle },
        { '?', 0, false, send_info },
};

ArduinoUno arduino_;
ArduinoSerial serial(Serial);
RomiSerial romiSerial(serial, serial, handlers, sizeof(handlers) / sizeof(MessageHandler));

static char reply_string[80];

static inline bool check_zero_index(int axis, IEncoder& encoder)
{
        bool index = false;
        if (encoder.get_index()) {
                index = true;
                stepper_zero_axis(axis);
                encoder.set_zero();
                encoder.reset_index();
        }
        return index;
}

void setup()
{
        disable_driver();
        
        Serial.begin(115200);
        while (!Serial)
                ;
        
        arduino_.setup();
        arduino_.init_encoders(1024, 1, 1); // TODO TODO

        // init_block_buffer();
        init_pins();
        init_stepper(&arduino_);

        controller_state = STATE_RUNNING;
        enable_stepper_timer();
}

static unsigned long last_time = 0;
static unsigned long last_print_time = 0;
static int16_t id = 0;
static bool print_ = false;

void loop()
{
        romiSerial.handle_input();

        // if (check_zero_index(0, arduino_.left_encoder()))
        //         romiSerial.log("LEFT=0");

        // if (check_zero_index(1, arduino_.right_encoder()))
        //         romiSerial.log("RIGHT=0");

        delay(1);

        if (print_) {
                print_perhaps();
        }
}

void print_perhaps()
{
        unsigned long now = millis();
        if (now - last_print_time > 1000) {
                print_status();
                last_print_time = now;
        }
}

void print_status()
{
        int16_t left_encoder_target;
        int16_t right_encoder_target;
        get_encoder_target(left_encoder_target, right_encoder_target);
        
        int32_t left_stepper_target;
        int32_t right_stepper_target;
        get_stepper_target(left_stepper_target, right_stepper_target);
        
        snprintf(reply_string, sizeof(reply_string),
                 "[enc(%ld,%ld),fltr(%ld,%ld),step(%ld,%ld),Te(%ld,%ld),Ts(%ld,%ld)]",
                 (int32_t) arduino_.left_encoder().get_position(),
                 (int32_t) arduino_.right_encoder().get_position(),
                 (int32_t) left_encoder_value_,
                 (int32_t) right_encoder_value_,
                 (int32_t) get_stepper_left(),
                 (int32_t) get_stepper_right(),
                 (int32_t) left_encoder_target,
                 (int32_t) right_encoder_target,
                 (int32_t) left_stepper_target,
                 (int32_t) right_stepper_target);
        romiSerial.log(reply_string);
}

static void reset()
{
        stepper_reset();
        arduino_.left_encoder().set_zero();
        arduino_.left_encoder().reset_index();
        arduino_.right_encoder().set_zero();
        arduino_.right_encoder().reset_index();
}

static void moveto(int16_t left, int16_t right)
{
        int32_t left_encoder = left * encoder_steps_per_revolution / 3600; 
        int32_t right_encoder = right * encoder_steps_per_revolution / 3600; 
        set_encoder_target(left_encoder, right_encoder);
                
        int32_t left_stepper = left * stepper_steps_per_revolution / 3600; 
        int32_t right_stepper = right * stepper_steps_per_revolution / 3600; 
        set_stepper_target(left_stepper, right_stepper);
}

void handle_moveto(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (controller_state == STATE_RUNNING
            || controller_state == STATE_PAUSED) {

                int32_t left = constrain(args[0], -1800, 1800);
                int32_t right = constrain(args[1], -1800, 1800);
                moveto(left, right);
                romiSerial->send_ok();  
                
        } else {
                romiSerial->send_error(101, kInvalidState);  
        }
}

void handle_pause(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (controller_state == STATE_RUNNING) {
                disable_stepper_timer();
                controller_state = STATE_PAUSED;
                romiSerial->send_ok();  
        } else if (controller_state == STATE_PAUSED) {
                romiSerial->send_ok();  
        } else {
                romiSerial->send_error(101, kInvalidState);  
        }
}

void handle_continue(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (controller_state == STATE_PAUSED) {
                controller_state = STATE_RUNNING;
                enable_stepper_timer();
                romiSerial->send_ok();  
        } else if (controller_state == STATE_RUNNING) {
                romiSerial->send_ok();  
        } else {
                romiSerial->send_error(101, kInvalidState);  
        }
}

void send_position(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        int32_t left_encoder = arduino_.left_encoder().get_position();
        int32_t right_encoder = arduino_.right_encoder().get_position();
        
        snprintf(reply_string, sizeof(reply_string),
                 "[0,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld]",
                 get_stepper_left(), get_stepper_right(),
                 left_encoder, right_encoder,
                 left_encoder_value_, right_encoder_value_,
                 (3600 * left_encoder) / encoder_steps_per_revolution,
                 (3600 * right_encoder) / encoder_steps_per_revolution);
        
        romiSerial->send(reply_string); 
}

// angle is in 10th of a degree (ex. 10° -> angle=100)
bool seek_index(int axis, int angle, IEncoder& encoder)
{
        bool found = false;

        // Serial.print("seek_index ");
        // Serial.println(angle);

        int32_t start = get_stepper_position(axis);
        int32_t end = angle * stepper_steps_per_revolution / 3600;
        int8_t direction = (end > start)? 1 : -1;
        
        if (axis == 0)
                moveto(angle, 0);
        else
                moveto(0, angle);
        
        while (true) {
                romiSerial.handle_input();
                
                int32_t current = get_stepper_position(axis);
                
                // Serial.print(axis);
                // Serial.print(": ");
                // Serial.print(current);
                // Serial.print(" ");
                // Serial.print(start);
                // Serial.print(" ");
                // Serial.print(end);
                // // Serial.print(" ");
                // // Serial.println(direction);
                // Serial.println();

                if (check_zero_index(axis, encoder)) {
                        // Serial.println("FOUND");
                        stepper_reset_axis(axis);
                        found = true;
                        break;
                }
                if ((direction > 0 && current >= end)
                    || (direction < 0 && current <= end)) {
                        break;
                }
                
                delay(1);
        }
        
        // Serial.print("seek_index ");
        // Serial.println(found);
        return found;
}

bool do_homing_axis(int axis, IEncoder& encoder)
{
        bool success = false;
        if (seek_index(axis, 20, encoder)  // 2°
            || seek_index(axis, -225, encoder)  // -22.5°
            || seek_index(axis, 3600, encoder)) { // 360°
                success = true;
        }
        return success;
}

bool do_homing()
{
        ControlMode backup_mode = mode_; // FIXME: use getter/setter
        mode_ = kOpenLoopControl;
        bool success = (do_homing_axis(0, arduino_.left_encoder())
                        && do_homing_axis(1, arduino_.right_encoder()));
        mode_ = backup_mode;  // FIXME: use getter/setter
        return success;
}

void handle_homing(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        controller_state = STATE_HOMING;
        
        reset();
        
        // Make sure the timer is running
        enable_stepper_timer();

        // Reply the client immediately because there's a timeout on
        // sending a response.
        romiSerial->send_ok();
        
        if (do_homing()) {
                controller_state = STATE_RUNNING;
        } else {
                controller_state = STATE_ERROR;
        }
}

void handle_enable(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (args[0] == 0) {
                disable_driver();
        } else {
                enable_driver()
        }
        romiSerial->send_ok();
}

void handle_print(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (args[0] == 0) {
                print_ = false;
        } else {
                print_ = true;
        }
        romiSerial->send_ok();
}

void change_mode(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        // FIXME: use getter/setter
        if (args[0] == kOpenLoopControl) {
                mode_ = kOpenLoopControl;
                romiSerial->send_ok();
        } else if (args[0] == kClosedLoopControl) {
                mode_ = kClosedLoopControl;
                romiSerial->send_ok();
        } else {
                romiSerial->send_error(1, "Unknown mode");  
        }
}

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send("[0,\"StepperSteering\",\"0.1\",\"" __DATE__ " " __TIME__ "\"]"); 
}

static inline bool is_idle()
{
        return controller_state == STATE_RUNNING; // Duh
}

void send_idle(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        snprintf(reply_string, sizeof(reply_string), "[0,%d,\"%c\"]",
                 is_idle(), controller_state);
        romiSerial->send(reply_string); 
}
