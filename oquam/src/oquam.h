#ifndef _ROMI_OQUAH_H_
#define _ROMI_OQUAH_H_

/**
 *
 */

typedef struct _controller_t controller_t;

typedef int (*controller_get_pos_t)(controller_t *controller, float *p);
typedef void (*controller_moveat_t)(controller_t *controller, float *v);
typedef void (*controller_delete_t)(controller_t *controller);

struct _controller_t {
        float vmax[6];
        float amax[6];
        float scale[6];
        controller_get_pos_t position;
        controller_moveat_t moveat;
        controller_delete_t del;
};

void controller_set_scale(controller_t *controller, float *scale);

/**
 *  \brief Sets the maximum speed.
 *
 *         Sets the maximum speed that the motors and the controller
 *         can handle. The speed is expressed in m/s for the XYZ axes
 *         and radian/sec for the ABC axes.
 */
void controller_set_vmax(controller_t *controller, float *vmax);

/**
 *  \brief Sets the maximum speed.
 *
 *         Sets the maximum acceleration that the motors and the
 *         controller can handle. The speed is expressed in m/s^2 for
 *         the XYZ axes and radian/s^2 for the ABC axes.
 */
void controller_set_amax(controller_t *controller, float *amax);

/**
 *  \brief Returns the current position.
 *
 *         Returns the current position in meters and radian. If the
 *         CNC is moving, the position is the last known position of
 *         the controller. In reality, the position is most likely
 *         different.
 */
int controller_get_position(controller_t *controller, float *pos);

/**
 *  \brief Move at a given speed.
 *
 *         Move at a given speed. The speed is given in m/s for the
 *         XYZ and radian/s for ABC.  
 */
void controller_moveat(controller_t *controller, float *v);

/**
 *  \brief Delete the controller and shut down the power.
 */
void delete_controller(controller_t *controller);


/**
 *
 */

controller_t *new_virtual_controller();

/**
 *
 */

typedef struct _cnc_t cnc_t;
typedef void (*cnc_callback_t)(void *userdata, cnc_t *cnc);

cnc_t *new_cnc();
void delete_cnc(cnc_t *cnc);

int cnc_get_position(cnc_t *cnc, float *p);

int cnc_begin_path(cnc_t *cnc);
void cnc_set_speed(cnc_t *cnc, float v);
int cnc_moveto(cnc_t *cnc, float x, float y, float z, float a, float b, float c);
int cnc_delay(cnc_t *cnc, float seconds);
int cnc_callback(cnc_t *cnc, cnc_callback_t cb, void *userdata);
int cnc_end_path(cnc_t *cnc);
int cnc_run_path(cnc_t *cnc);

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // _ROMI_OQUAH_H_
