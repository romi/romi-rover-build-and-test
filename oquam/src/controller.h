
#ifndef _OQUAM_CONTROLLER_H_
#define _OQUAM_CONTROLLER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _cnc_t cnc_t;
typedef struct _planner_t planner_t;
typedef struct _controller_t controller_t;

typedef int (*controller_run_t)(controller_t *controller, cnc_t *cnc,
                                planner_t *planner, int async);
typedef int (*controller_position_t)(controller_t *controller, double *p);
typedef int (*controller_moveat_t)(controller_t *controller, double *v);
typedef void (*controller_delete_t)(controller_t *controller);

struct _controller_t {
        /**
         * The maximum positions, in m
         */
        double xmax[3];

        /**
         * The maximum speed, in m/s
         */
        double vmax[3];

        /**
         * The maximum acceleration, in m/s^2
         */
        double amax[3];

        controller_run_t run;
        controller_position_t position;
        controller_moveat_t moveat;
        controller_delete_t del;
};

/**
 *  \brief Returns the current position.
 *
 *         Returns the current position in meters and radian. If the
 *         CNC is moving, the position is the last known position of
 *         the controller. In reality, the position is most likely
 *         different.
 */
int controller_position(controller_t *controller, double *pos);

/**
 *  \brief Move at a given speed.
 *
 *         Move at a given speed. The speed is given in m/s for the
 *         XYZ and radian/s for ABC.  
 */
int controller_moveat(controller_t *controller, double *v);

/**
 *  \brief Move at a given speed.
 *
 *         Move at a given speed. The speed is given in m/s for the
 *         XYZ and radian/s for ABC.  
 */
int controller_run(controller_t *controller, cnc_t *cnc, planner_t *planner, int async);

/**
 *  \brief Delete the controller and shut down the power.
 */
void delete_controller(controller_t *controller);


#ifdef __cplusplus
}
#endif

#endif // _OQUAM_CONTROLLER_H_
