#include "trigger.h"

#define TRIGGER_BUFFER_SIZE 8
#define TRIGGER_BUFFER_SIZE_MASK 0x07

/**
 * \brief The 'trigger_buffer' is a circular buffer of trigger IDs.
 */
typedef struct _trigger_buffer_t {
        uint8_t readpos;
        uint8_t writepos;
        int16_t trigger[TRIGGER_BUFFER_SIZE];
} trigger_buffer_t;

static trigger_buffer_t trigger_buffer;

void init_trigger_buffer()
{
        trigger_buffer.readpos = 0;
        trigger_buffer.writepos = 0;
}

int trigger_buffer_put(int16_t id)
{
        /* This function is called in the stepper timer's ISR so it
         * has to be quick. It should not get disturbed by any other
         * code (no other ISR accesses the trigger buffer). */        
        uint8_t n = ((TRIGGER_BUFFER_SIZE
                      - trigger_buffer.writepos - 1
                      + trigger_buffer.readpos)
                     & TRIGGER_BUFFER_SIZE_MASK);
        
        if (n == 0)
                return -1;
        
        trigger_buffer.trigger[trigger_buffer.writepos] = id;
        
        // if (++trigger_buffer.writepos == TRIGGER_BUFFER_SIZE)
        //         trigger_buffer.writepos = 0;
        
        trigger_buffer.writepos = ((trigger_buffer.writepos + 1)
                                   & TRIGGER_BUFFER_SIZE_MASK);
        
        return 0;
}

static inline uint8_t _available()
{
        return ((TRIGGER_BUFFER_SIZE
                 - trigger_buffer.readpos
                 + trigger_buffer.writepos)
                & TRIGGER_BUFFER_SIZE_MASK);
}

uint8_t trigger_buffer_available()
{
        return _available();
}

int16_t trigger_buffer_get()
{
        /* The execution of this function may be interrupted by the
         * stepper timer timer. Take some care before incrementing the
         * readpos to make sure we have the value in a local variable
         * and that we don't temporarily set the readpos to an illegal
         * value such as TRIGGER_BUFFER_SIZE. */
        
        uint16_t v = trigger_buffer.trigger[trigger_buffer.readpos];
        uint8_t n = trigger_buffer.readpos + 1;
        if (n == TRIGGER_BUFFER_SIZE)
                trigger_buffer.readpos = 0;
        else 
                trigger_buffer.readpos = n;
        return v;
}
