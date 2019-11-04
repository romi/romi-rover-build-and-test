#include <stdint.h>

#ifndef _OQUAM_TRIGGER_H_
#define _OQUAM_TRIGGER_H_

/**
 * \brief Initializes the trigger buffer.
 */
void init_trigger_buffer();

/**
 * \brief Empties the buffer.
 */
#define trigger_buffer_clear() init_trigger_buffer()

/**
 * \brief Adds a trigger to the send list.
 *
 * If the buffer is full, the trigger will be discarded and -1 will be
 * returned. Otherwise, the id is inserted at the end of the buffer
 * and the function returns zero. 
 */
int trigger_buffer_put(int16_t id);

/**
 * \brief Returns the number of triggers waiting to be send.
 */
uint8_t trigger_buffer_available();

/**
 * \brief Returns the next id of the trigger to be send.
 *
 * You MUST verify that there is data in the buffer by calling
 * trigger_buffer_available() first.
 */
int16_t trigger_buffer_get();

#endif // _OQUAM_TRIGGER_H_
