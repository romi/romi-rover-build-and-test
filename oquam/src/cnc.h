/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */

#ifndef _OQUAM_CNC_H_
#define _OQUAM_CNC_H_

#include <stdint.h>
#include "controller.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _cnc_t cnc_t;
typedef void (*cnc_callback_t)(void *userdata, cnc_t *cnc, int16_t arg);

cnc_t *new_cnc(controller_t *controller);
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

int cnc_end_script(cnc_t *cnc);

int cnc_run_script(cnc_t *cnc, int async);

int cnc_plot_script(cnc_t *cnc, const char *filepath);


/**
 * \brief Resume the execution.
 */
int cnc_continue(cnc_t *cnc);

int cnc_get_position(cnc_t *cnc, double *p);

/**
 * \brief Directly move to a given position in a straight line.
 *
 * This function is provided for convenience. It is equivalent to the
 * following code:
 *
 *      cnc_begin_script(cnc, 0.0);
 *      cnc_move(cnc, x, y, z, v);
 *      cnc_end_script(cnc);
 *      cnc_run_script(cnc, 0);
 *
 * The CNC will move in a straight line taking into the maximum
 * acceleration/deceleration. It will come to a standstill at the end
 * of the traveling.
 *
 */
int cnc_moveto(cnc_t *cnc, double x, double y, double z, double v);


/**
 * \brief Instantly move at a given speed.
 *
 */
int cnc_moveat(cnc_t *cnc, double vx, double vy, double vz);

#ifdef __cplusplus
}
#endif

#endif // _OQUAM_CNC_H_
