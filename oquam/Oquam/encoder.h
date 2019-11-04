
#ifndef _OQUAM_ENCODER_H_
#define _OQUAM_ENCODER_H_

void init_encoders();
void handle_x_encoder_interrupt();
void handle_y_encoder_interrupt();

extern volatile int16_t encoder_position[3];

#endif // _OQUAM_ENCODER_H_
