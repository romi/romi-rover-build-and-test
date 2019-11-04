#include "action.h"
#include "trigger.h"
#include "stepper.h"
#include "pins.h"
#include "encoder.h"
#include "Parser.h"

/**
 *  \brief The possible states of the controller (the main thread).
 */
enum {
        STATE_INTERACTIVE,
        STATE_MOVING,
        STATE_WAITING_CORRECTION,
        STATE_WAITING_RESET
};

extern volatile uint8_t thread_state;
extern volatile int32_t counter_stepper_timer;
extern volatile int32_t counter_reset_timer;
extern volatile int16_t stepper_positions[3];

uint8_t controller_state;
uint8_t error_number;
Parser parser("BMDT", "?SCWQ");
uint8_t begin_at = 1;

// DEBUG
extern int16_t accumulation_error[3];

void send_state()
{
        if (trigger_buffer_available() > 0) {
                int16_t id = trigger_buffer_get();
                Serial.print("T");
                Serial.println(id);
                return;
        }

        uint8_t n = action_buffer_available();
        switch (thread_state) {
        case STATE_THREAD_EXECUTING:
                Serial.print("E");
                Serial.println((int) n);
                break;
        case STATE_THREAD_STARTING:
                Serial.print("B[");
                Serial.println((int) n);
                Serial.print(",");
                Serial.print(begin_at);
                Serial.println("]");
                break;
        case STATE_THREAD_IDLE:
                Serial.print("I");
                Serial.println((int) n);
                break;
        }
}

void check_state()
{
        // if (controller_state == STATE_MOVING) {
        //         if (thread_state == STATE_THREAD_IDLE) {
        //                 send_finished_path();
        //                 controller_state = STATE_INTERACTIVE;
                        
        //         } else if (thread_state == STATE_THREAD_CORRECTING) {
        //                 request_path_correction();
        //                 controller_state = STATE_WAITING_CORRECTION;
                        
        //         } else if (thread_state == STATE_THREAD_ERROR) {
        //                 send_error(error_number);
        //                 controller_state = STATE_WAITING_RESET;
        //         }
        // }
}

void start_stepper_thread_perhaps()
{
        if (thread_state == STATE_THREAD_STARTING
            && action_buffer_available() >= begin_at)
                enable_stepper_timer();
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
                                break;

                                /* First handle the action commands
                                 * 'M', 'D, 'T', 'W'.
                                 */
                                
                        case 'M':
                                if (parser.length() == 4
                                    && parser.value(0) > 0) {
                                        action_t *action = action_buffer_get_empty();
                                        if (action == 0) {
                                                Serial.println("RE");
                                        } else {
                                                action->type = ACTION_MOVE;
                                                action->data[DT] = parser.value(0);
                                                action->data[DX] = parser.value(1);
                                                action->data[DY] = parser.value(2);
                                                action->data[DZ] = parser.value(3);
                                                action_buffer_ready();
                                                start_stepper_thread_perhaps();
                                                Serial.println("OK");
                                        }
                                } else {
                                        Serial.println("ERR bad args");
                                }
                                break;
                        case 'D':
                                if (parser.length() == 1
                                    && parser.value(0) > 0) {
                                        action_t *action = action_buffer_get_empty();
                                        if (action == 0) {
                                                Serial.println("RE");
                                        } else {
                                                action->type = ACTION_DELAY;
                                                action->data[0] = parser.value(0);
                                                action_buffer_ready();
                                                start_stepper_thread_perhaps();
                                                Serial.println("OK");
                                        }
                                } else {
                                        Serial.println("ERR bad arg");
                                }
                                break;
                        case 'T':
                                if (parser.length() == 1) {
                                        action_t *action = action_buffer_get_empty();
                                        if (action == 0) {
                                                Serial.println("RE");
                                        } else {
                                                action->type = ACTION_TRIGGER;
                                                action->data[0] = parser.value(0);
                                                action_buffer_ready();
                                                start_stepper_thread_perhaps();
                                                Serial.println("OK");
                                        }
                                } else {
                                        Serial.println("ERR bad arg");
                                }
                                break;
                        case 'W':
                                if (1) {
                                        action_t *action = action_buffer_get_empty();
                                        if (action == 0) {
                                                Serial.println("RE");
                                        } else {
                                                action->type = ACTION_WAIT;
                                                action_buffer_ready();
                                                start_stepper_thread_perhaps();
                                                Serial.println("OK");
                                        }
                                } else {
                                        Serial.println("ERR bad arg");
                                }
                                break;
                        case 'C':
                                if (thread_state == STATE_THREAD_IDLE)
                                        enable_stepper_timer();
                                Serial.println("OK");
                                break;
                        case 'Q':
                                disable_stepper_timer();
                                thread_state = STATE_THREAD_IDLE;
                                Serial.println("OK");
                                break;
                        case 'B':
                                if (parser.length() == 1 && parser.value(0) > 0) {
                                        begin_at = parser.value(0);
                                        disable_stepper_timer();
                                        delay(2);
                                        action_buffer_clear();
                                        trigger_buffer_clear();
                                        thread_state = STATE_THREAD_STARTING;
                                        Serial.println("OK");
                                } else {
                                        Serial.println("ERR bad arg");
                                }
                                break;
                        case 'S':
                                send_state();
                                break;
                        case '?':
                                Serial.print("?[\"Oquam\",\"0.1\"]"); 
                                break;
                        }
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
        Serial.begin(115200);
        while (!Serial)
                ;

        controller_state = STATE_INTERACTIVE;
        
        init_action_buffer();
        init_trigger_buffer();
        init_output_pins();
        init_encoders();
        
        enable_driver();

        // FIXME: don't belong here
        for (int i = 0; i < 3; i++) {
                stepper_positions[i] = 0;
                accumulation_error[i] = 0;
        }
        
        cli();
        init_stepper_timer();
        init_reset_step_pins_timer();
        sei(); 

}

void loop()
{
        check_state();
        check_accuracy();
        handle_input();
        
        if (thread_state == STATE_THREAD_EXECUTING) {
                Serial.print(thread_state);
                Serial.print(" ");
                Serial.print(counter_stepper_timer);
                Serial.print(" ");
                Serial.print(accumulation_error[0]);
                Serial.print(" ");
                Serial.print(counter_reset_timer);
                Serial.print(" ");
                Serial.println(stepper_positions[0]);
                delay(100);
        } else {
                static unsigned long last_print = 0;
                if (millis() - last_print > 500) {
                        last_print = millis();
                        //send_state();
                        Serial.println(encoder_position[0]);
                }
        }
}
