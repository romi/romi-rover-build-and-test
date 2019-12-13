#include "block.h"
#include "trigger.h"
#include "stepper.h"
#include "pins.h"
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
extern volatile int16_t stepper_position[3];
extern volatile int16_t milliseconds;
extern volatile int16_t block_id;
extern volatile int16_t block_ms;
extern volatile int8_t stepper_reset;
extern volatile int16_t encoder_position[3];

uint8_t controller_state;
uint8_t error_number;
Parser parser("MDTV", "?SCWXP");

// DEBUG
extern int32_t accumulation_error[3];

static char state_string[32];
static char position_string[50];

char* get_state_string()
{
        uint8_t n = block_buffer_available();
        char *s;
        
        switch (thread_state) {
        case STATE_THREAD_EXECUTING:
                s = "e";
                break;
        case STATE_THREAD_IDLE:
                s = "i";
                break;
        }
        
        snprintf(state_string, sizeof(state_string),
                 "['%s',%d,%d,%d,%d,%d]",
                 s, (int) n, (int) block_id, (int) block_ms,
                 (int) milliseconds, (int) interrupts);
        return state_string;
}

void send_state()
{
        if (trigger_buffer_available() > 0) {
                int16_t id = trigger_buffer_get();
                Serial.print("T");
                Serial.println(id);
        } else {
                Serial.print("S");
                Serial.println(get_state_string());
        }
}

char* get_position_string()
{
        snprintf(position_string, sizeof(position_string),
                 "[%d,%d,%d,%d,%d,%d,%lu]",
                 (int) stepper_position[0],
                 (int) stepper_position[1],
                 (int) stepper_position[2],
                 (int) encoder_position[0],
                 (int) encoder_position[1],
                 (int) encoder_position[2],
                 millis());
        return position_string;
}

void send_position()
{
        Serial.print("P");
        Serial.println(get_position_string());
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
                                
                                /* First handle the commands 'M', 'V',
                                 * 'D, 'T', 'W' to create a new block.
                                 */
                                
                        case 'M':
                                if (parser.length() >= 4
                                    && parser.value(0) > 0) {
                                        block_t *block = block_buffer_get_empty();
                                        if (block == 0) {
                                                Serial.print("RE ");
                                                Serial.println(get_state_string());
                                        } else {
                                                block->type = BLOCK_MOVE;
                                                block->data[DT] = parser.value(0);
                                                block->data[DX] = parser.value(1);
                                                block->data[DY] = parser.value(2);
                                                block->data[DZ] = parser.value(3);
                                                if (parser.length() == 5)
                                                        block->id = parser.value(4);
                                                block_buffer_ready();
                                                Serial.print("OK ");
                                                Serial.println(get_state_string());
                                        }
                                } else if (parser.value(0) <= 0) {
                                        // Zero duration: just skip
                                        Serial.print("OK ");
                                        Serial.println(get_state_string());
                                } else if (parser.length() < 4) {
                                        Serial.println("ERR missing args");
                                }
                                break;
                        case 'V':
                                if (parser.length() >= 3) {
                                        block_t *block = block_buffer_get_empty();
                                        if (block == 0) {
                                                Serial.print("RE ");
                                                Serial.println(get_state_string());
                                        } else {
                                                block->type = BLOCK_MOVEAT;
                                                block->data[DT] = 1000;
                                                block->data[DX] = parser.value(0);
                                                block->data[DY] = parser.value(1);
                                                block->data[DZ] = parser.value(2);
                                                if (parser.length() == 4)
                                                        block->id = parser.value(3);
                                                block_buffer_ready();
                                                Serial.print("OK ");
                                                Serial.println(get_state_string());
                                        }
                                } else if (parser.length() < 3) {
                                        Serial.println("ERR missing args");
                                }
                                break;
                        case 'D':
                                if (parser.length() == 1
                                    && parser.value(0) > 0) {
                                        block_t *block = block_buffer_get_empty();
                                        if (block == 0) {
                                                Serial.println("RE");
                                        } else {
                                                block->type = BLOCK_DELAY;
                                                block->data[0] = parser.value(0);
                                                block_buffer_ready();
                                                Serial.print("OK ");
                                                Serial.println(block_buffer_available());
                                        }
                                } else {
                                        Serial.println("ERR bad arg");
                                }
                                break;
                        case 'T':
                                if (parser.length() == 1
                                    || parser.length() == 2) {
                                        int16_t id, arg = 0;
                                        id = parser.value(2);
                                        if (parser.length() == 2)
                                                arg = parser.value(1);
                                        block_t *block = block_buffer_get_empty();
                                        if (block == 0) {
                                                Serial.println("RE");
                                        } else {
                                                block->type = BLOCK_TRIGGER;
                                                block->data[0] = id;
                                                block->data[1] = arg;
                                                block_buffer_ready();
                                                Serial.print("OK ");
                                                Serial.println(block_buffer_available());
                                        }
                                } else {
                                        Serial.println("ERR bad arg");
                                }
                                break;
                        case 'W':
                                if (1) {
                                        block_t *block = block_buffer_get_empty();
                                        if (block == 0) {
                                                Serial.println("RE");
                                        } else {
                                                block->type = BLOCK_WAIT;
                                                block_buffer_ready();
                                                Serial.print("OK ");
                                                Serial.println(block_buffer_available());
                                        }
                                } else {
                                        Serial.println("ERR bad arg");
                                }
                                break;

                                /* The other, 'real-time', commands. */
                                
                        case 'C':
                                enable_stepper_timer();
                                Serial.println("OK Cont");
                                break;
                        case 'S':
                                send_state();
                                break;
                        case 'P':
                                send_position();
                                break;
                        case 'X':
                                stepper_reset = 1;
                                block_buffer_clear();
                                trigger_buffer_clear();
                                Serial.println("OK Clear");
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

                if (time - last_print_time > 50) {
                        Serial.print(block_id);
                        Serial.print(":");
                        Serial.print(milliseconds);
                        Serial.print(":");
                        Serial.print(interrupts);
                        Serial.print(":");
                        Serial.print(block_buffer_readpos());
                        Serial.print(":");
                        Serial.print(block_buffer_writepos());
                        Serial.println();
                        last_print_time = time;
                }
        }
}

