#include "Arduino.h"
#include "pins.h"

/**
 * \brief Configure the step and dir pins as output.
 */
void init_output_pins() 
{
        /* Enable output on the selected pins */
        STEP_DDR |= STEP_MASK;
        DIRECTION_DDR |= DIRECTION_MASK;
        STEPPERS_DISABLE_DDR |= (1 << STEPPERS_DISABLE_BIT);

        /* X encoder */
        pinMode(X_ENCODER_A, INPUT_PULLUP);
        pinMode(X_ENCODER_B, INPUT_PULLUP);
 
        /* Y encoder */
        pinMode(Y_ENCODER_A, INPUT_PULLUP);
        pinMode(Y_ENCODER_B, INPUT_PULLUP);
}
