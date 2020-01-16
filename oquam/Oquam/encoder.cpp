#include <avr/interrupt.h>
#include "Arduino.h"
#include "config.h"
#include "encoder.h"

volatile int32_t encoder_position[3];

void init_encoders()
{
        for (int i = 0; i < 3; i++)
                encoder_position[i] = 0;

        if (X_ENCODER_A > 0) {
                pinMode(X_ENCODER_A, INPUT_PULLUP);
                pinMode(X_ENCODER_B, INPUT_PULLUP);
                attachInterrupt(digitalPinToInterrupt(X_ENCODER_A),
                                handle_x_encoder_interrupt,
                                RISING);
        }
        
        if (Y_ENCODER_A > 0) {
                pinMode(Y_ENCODER_A, INPUT_PULLUP);
                pinMode(Y_ENCODER_B, INPUT_PULLUP);
                attachInterrupt(digitalPinToInterrupt(Y_ENCODER_A),
                                handle_y_encoder_interrupt,
                                RISING);
        }
}

/**
 * Interrupt service routines for the quadrature encoder on the
 * X-axis.
 */
void handle_x_encoder_interrupt()
{
        uint8_t b = get_x_encoder_b();
#if ENCODER_REVERSED
        encoder_position[0] -= b ? -1 : +1;
#else
        encoder_position[0] += b ? -1 : +1;
#endif
}
 
/**
 * Interrupt service routines for the quadrature encoder on the
 * Y-axis.
 */
void handle_y_encoder_interrupt()
{
        uint8_t b = get_y_encoder_b(); 
#if ENCODER_REVERSED
        encoder_position[1] -= b ? -1 : +1;
#else
        encoder_position[1] += b ? -1 : +1;
#endif
}
