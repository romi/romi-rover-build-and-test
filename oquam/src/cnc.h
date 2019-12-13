
#ifndef _OQUAM_CNC_H_
#define _OQUAM_CNC_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _cnc_t cnc_t;
typedef void (*cnc_callback_t)(void *userdata, cnc_t *cnc, int16_t arg);

cnc_t *new_cnc();
void delete_cnc(cnc_t *cnc);

int cnc_begin_script(cnc_t *cnc, double d);

/**
 * \brief Add a move action to the current script.
 *
 * Move to position (x,y,z) in meters at a speed of v m/s.
 */
int cnc_move(cnc_t *cnc, double x, double y, double z, double v);

/**
 * \brief Add a delay instruction to the current script.
 *
 * \brief Delay the execution for a given number of seconds.
 */
int cnc_delay(cnc_t *cnc, double seconds);

/**
 * \brief Add a trigger action to the current script.
 *
 * Triggers a callback.
 */
int cnc_trigger(cnc_t *cnc, cnc_callback_t cb, void *userdata, int arg);

/**
 * \brief Add a pause instruction to the current script.
 *
 * Pauses the execution. 
 */
int cnc_wait(cnc_t *cnc);

/**
 * \brief Resume the execution.
 */
int cnc_continue(cnc_t *cnc);

int cnc_end_script(cnc_t *cnc);
int cnc_plot_script(cnc_t *cnc, const char *filepath);
int cnc_run_script(cnc_t *cnc, int async);

int cnc_get_position(cnc_t *cnc, double *p);

#ifdef __cplusplus
}
#endif

#endif // _OQUAM_CNC_H_
