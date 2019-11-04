#include "action.h"

/**
 *  Should be smaller or equal to 256.
 */
#define ACTION_BUFFER_SIZE 128
#define ACTION_BUFFER_SIZE_MASK 0x7f

/**
 * \brief The 'action_buffer' is a circular buffer of actions.
 */
typedef struct _action_buffer_t {
        uint8_t readpos;
        uint8_t writepos;
        action_t action[ACTION_BUFFER_SIZE];
} action_buffer_t;

action_buffer_t action_buffer;

void init_action_buffer()
{
        action_buffer.readpos = 0;
        action_buffer.writepos = 0;
}

static inline uint8_t _space()
{
        // if (action_buffer.writepos >= action_buffer.readpos)
        //         return (ACTION_BUFFER_SIZE
        //                 - action_buffer.writepos - 1
        //                 + action_buffer.readpos);
        // else
        //         return action_buffer.readpos - action_buffer.writepos - 1;

        // The following optimization takes advantage from the fact
        // that the buffer size is a power of two.
        return (ACTION_BUFFER_SIZE
                - action_buffer.writepos - 1
                + action_buffer.readpos) & ACTION_BUFFER_SIZE_MASK;
}

uint8_t action_buffer_space()
{
        return _space();
}

action_t *action_buffer_get_empty()
{
        if (_space() == 0)
                return 0;
        return &action_buffer.action[action_buffer.writepos];
}

void action_buffer_ready()
{
        action_buffer.writepos++;
}

static inline uint8_t _available()
{
        // if (action_buffer.writepos >= action_buffer.readpos)
        //         return action_buffer.writepos - action_buffer.readpos;
        // else 
        //         return (ACTION_BUFFER_SIZE
        //                 - action_buffer.readpos
        //                 + action_buffer.writepos) & 0x7f;

        // The following optimization takes advantage from the fact
        // that the buffer size is a power of two.
        return (ACTION_BUFFER_SIZE
                - action_buffer.readpos
                + action_buffer.writepos) & ACTION_BUFFER_SIZE_MASK;
}

uint8_t action_buffer_available()
{
        return _available();
}

action_t *action_buffer_get_next()
{
        if (_available() == 0)
                return 0;
        return &action_buffer.action[action_buffer.readpos++];
}
