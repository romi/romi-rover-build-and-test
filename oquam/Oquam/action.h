/**
 * The files action.c and action.cpp define the action_t data
 * structure and a set of function to manipulate the action
 * buffer. The action buffer is a circular buffer. It only handle one
 * writer (the main thread) and one reader (the interrupt-driver
 * stepper thread).
 *
 */
#include <stdint.h>

#ifndef _OQUAM_ACTION_H_
#define _OQUAM_ACTION_H_

/**
 * \brief The possible action types.
 */
enum {
        ACTION_WAIT = 0,
        ACTION_MOVE,
        ACTION_DELAY,
        ACTION_TRIGGER
};

/**
 * \brief The data fields for the move action.
 */
enum {
        DT = 0,
        DX = 1,
        DY = 2,
        DZ = 3
};

/**
 * \brief The 'action' structure contains the information of one
 * action.
 *
 * There are three types of action that the controller can exexute:
 *
 * 1. stop: Come to a standstill and go into idle mode. There's no
 *    extra data.
 *
 * 2. move: The data contains the number timer interrupts (data[0]),
 *    the number of steps to move in each of the three directions
 *    (data[1-3]).
 *
 * 3. delay: The controller should pause for a given delay. The delay
 *    is given in data[0] and is expressed in milliseconds.
 *
 * 4. trigger: Request to send a trigger to the controlling
 *    program. The trigger ID is stored in data[0];
 *
 * 5. trigger: Request to execute a callback by the controlling
 *    program. The callback ID is stored in data[0];
 */
typedef struct _action_t {
        uint8_t type;
        int16_t id;
        int16_t data[4];
} action_t;

/**
 * \brief Initializes the action buffer.
 */
void init_action_buffer();

/**
 * \brief Empties the buffer.
 */
#define action_buffer_clear() init_action_buffer()

/**
 * \brief Returns the number of empty action slots in the buffer.
 */
uint8_t action_buffer_space();

/**
 * \brief Returns a pointer to the next of empty action slot but
 * doesn't mark the slot as ready.
 *
 * Returns NULL if there are no empty action slots in the buffer.
 */
action_t *action_buffer_get_empty();

/**
 * \brief Marks the next empty slot as ready.
 *
 * This function should be called after having obtained an action slot
 * using action_buffer_get_empty() and when the action is initialized.
 */
void action_buffer_ready();

/**
 * \brief Returns the number of actions waiting to be executed.
 */
uint8_t action_buffer_available();

/**
 * \brief Returns a pointer to the next of action to be executed.
 *
 * Returns 0 if there are no more actions to be executed in the
 * buffer.
 */
action_t *action_buffer_get_next();

#endif // _OQUAM_ACTION_H_
