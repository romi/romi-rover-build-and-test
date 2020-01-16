#include "Arduino.h"
#include "config.h"

/**
 * \brief Configure the step and dir pins as output.
 */
void init_output_pins() 
{
        /* Enable output on the selected pins */
        STEP_DDR |= STEP_MASK;
        DIRECTION_DDR |= DIRECTION_MASK;
        STEPPERS_DISABLE_DDR |= STEPPERS_DISABLE_MASK;

        LIMIT_DDR &= ~(LIMIT_MASK); // Set as input pins
        LIMIT_PORT |= (LIMIT_MASK);  // Enable internal pull-up resistors. Normal high operation        
        /* The pins for the encoders are initialized in encoder.cpp */
}
