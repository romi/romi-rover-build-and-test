
#include <r.h>
#include "oquam.h"

enum {
        X = 0, Y, Z,
        A, B, C
};

enum {
        STOP = 0,
        MOVE,
        DELAY,
        TRIGGER
};



typedef struct _action_t action_t;

struct _action_t {
        unsigned char type;
        float pos[6];
        float v;
        float delay;
        void *callback;
        void *userdata;
        action_t *next;
        action_t *prev;
};

action_t *new_action(int type)
{
        action_t *action = r_new(action_t);
        if (action == NULL)
                return NULL;
        action->type = type;
        return action;
}

action_t *new_delay(float delay)
{
        action_t *action = new_action(DELAY);
        if (action == NULL)
                return NULL;
        action->delay = delay;
        return action;
}

action_t *new_move(float x, float y, float z,
                   float a, float b, float c,
                   float v)
{
        action_t *action = new_action(DELAY);
        if (action == NULL)
                return NULL;
        action->pos[0] = x;
        action->pos[1] = y;
        action->pos[2] = z;
        action->pos[3] = a;
        action->pos[4] = b;
        action->pos[5] = c;
        action->v = v;
        return action;
}

action_t *new_callback(void *callback, void *userdata)
{
        action_t *action = new_action(DELAY);
        if (action == NULL)
                return NULL;
        action->callback = callback;
        action->userdata = userdata;
        return action;
}

void delete_action(action_t *action)
{
        if (action) {
                r_delete(action);
        }
}

void action_append(action_t *action, action_t *next)
{
        action->next = next;
        next->prev = action;
}

action_t *action_prev(action_t *action)
{
        return action->prev;
}

action_t *action_next(action_t *action)
{
        return action->next;
}

/**
 *
 */
typedef struct _path_t {
        action_t *first;
        action_t *last;
} path_t;

path_t *new_path()
{
        path_t *path = r_new(path_t);
        if (path == NULL)
                return NULL;
        return path;
}

void delete_path(path_t *path)
{
        if (path) {
                action_t *s = path->first;
                while (s) {
                        action_t *n = s->next;
                        delete_action(s);
                        s = n;
                }
                r_delete(path);
        }
}

void path_append(path_t *path, action_t *action)
{
        if (path->last == NULL) {
                path->first = action;
                path->last = action;
        } else {
                action_append(path->last, action);
        }
}

action_t *path_first(path_t *path)
{
        return path->first;
}

action_t *path_last(path_t *path)
{
        return path->last;
}


/**
 *
 */

void controller_set_scale(controller_t *controller, float *scale)
{
        memcpy(controller->scale, scale, sizeof(controller->scale)); 
}

void controller_set_vmax(controller_t *controller, float *vmax)
{
        memcpy(controller->vmax, vmax, sizeof(controller->vmax)); 
}

void controller_set_amax(controller_t *controller, float *amax)
{
        memcpy(controller->amax, amax, sizeof(controller->amax)); 
}

int controller_get_position(controller_t *controller, float *pos)
{
        if (controller->position(controller, pos) != 0)
                return -1;
        for (int i = 0; i < 6; i++)
                pos[i] /= controller->scale[i];
        return 0;
}

void controller_moveat(controller_t *controller, float *v)
{
        float vs[6];
        for (int i = 0; i < 6; i++)
                vs[i] = v[i] * controller->scale[i];        
        controller->moveat(controller, vs);
}

void delete_controller(controller_t *controller)
{
        if (controller->del)
                controller->del(controller);
}

/**
 *
 */

typedef struct _virtual_controller_t {
        controller_t interface;
        float pos[6];
        float v[6];
        int finished;
        thread_t *thread;
        mutex_t *mutex;
} virtual_controller_t;

int virtual_controller_get_position(controller_t *controller, float *p);
void virtual_controller_moveat(controller_t *controller, float *v);
void delete_virtual_controller(controller_t *controller);
void virtual_controller_run(void* data);

controller_t *new_virtual_controller()
{
        virtual_controller_t *controller = r_new(virtual_controller_t);
        if (controller == NULL)
                return NULL;
        
        controller->interface.position = virtual_controller_get_position;
        controller->interface.moveat = virtual_controller_moveat;
        controller->interface.del = delete_virtual_controller;
        
        controller->finished = 0;

        controller->mutex = new_mutex();
        if (controller->mutex == NULL) {
                delete_virtual_controller((controller_t *) controller);
                return NULL;
        }

        controller->thread = new_thread(virtual_controller_run, controller, 1, 0);
        if (controller->thread == NULL) {
                delete_virtual_controller((controller_t *) controller);
                return NULL;
        }

        return (controller_t*) controller;
}

void delete_virtual_controller(controller_t *controller)
{
        virtual_controller_t *c = (virtual_controller_t*) controller;
        if (c) {
                c->finished = 1;
                if (c->thread) {
                        thread_join(c->thread);
                        delete_thread(c->thread);
                }
                if (c->mutex)
                        delete_mutex(c->mutex);
                r_delete(c);
        }
}

int virtual_controller_get_position(controller_t *controller, float *p)
{
        virtual_controller_t *c = (virtual_controller_t*) controller;
        memcpy(p, c->pos, sizeof(c->pos));
        return 0;
}

void virtual_controller_moveat(controller_t *controller, float *v)
{
        virtual_controller_t *c = (virtual_controller_t*) controller;
        memcpy(c->v, v, sizeof(c->v));
}

void virtual_controller_run(void* data)
{
        virtual_controller_t *c = (virtual_controller_t*) data;

        double last_t = clock_time();
        while (!c->finished) {
                mutex_lock(c->mutex);
                double t = clock_time();
                double dt = t - last_t;
                for (int i = 0; i < 6; i++)
                        c->pos[i] += c->v[i] * dt;
                mutex_unlock(c->mutex);
                last_t = t;
                clock_sleep(0.005);
        }
}

/**
 *
 */

enum {
        /** Checks current position against expected position and
         * sends the host the distance between the two. */
        CPOINT_POS = 0,
        /** Sets the current velocity. */
        CPOINT_VEL,
        /** Sets the current acceleration. */
        CPOINT_ACCEL,
        /** Speed and acceleration are set to zero and the CNC waits
         * for a certain amount of time. */
        CPOINT_DELAY,
        /** Tell the host to execute a callback. */
        CPOINT_CALLBACK
};

typedef struct _cpoint_t {
        unsigned char type;
        uint16_t pos[3];
        uint16_t data[3];
} cpoint_t;

struct _callback_t {
        cnc_callback_t callback;
        void *userdata;
        cnc_t *cnc;
} callback_t;



/**
 *
 */
typedef struct _planner_t {
        cpoint_t *cpoints;
        int num_cpoints;
        callback_t *callbacks;
        int num_callbacks;
} planner_t;

planner_t *new_planner(cnc_t *cnc, controller_t *controller, path_t *path)
{
        planner_t *planner = r_new(planner_t);
        if (planner == NULL)
                return NULL;
        return planner;
}

void delete_planner(planner_t *planner)
{
        if (planner) {
                
                r_delete(planner);
        }
}


/**
 *
 */

struct _cnc_t {
        float v;
        controller_t *controller;
        planner_t *planner;
        path_t *path;
};

cnc_t *new_cnc(controller_t *controller)
{
        cnc_t *cnc = r_new(cnc_t);
        if (cnc == NULL)
                return NULL;
        cnc->controller = controller;
        
        cnc->planner = new_planner();
        if (cnc->planner == NULL) {
                
        }
        return cnc;
}

void delete_cnc(cnc_t *cnc)
{
        if (cnc) {
                if (cnc->path != NULL)
                        delete_path(cnc->path);
                if (cnc->planner != NULL)
                        delete_planner(cnc->planner);
                if (cnc->controller != NULL)
                        delete_controller(cnc->controller);
                r_delete(cnc);
        }
}

int cnc_begin_path(cnc_t *cnc)
{
        if (cnc->path != NULL) {
                delete_path(cnc->path);
                cnc->path = NULL;
        }
        cnc->path = new_path();
        if (cnc->path == NULL)
                return -1;
        return 0;
}

void cnc_set_speed(cnc_t *cnc, float v)
{
        cnc->v = v;
}

int cnc_moveto(cnc_t *cnc, float x, float y, float z, float a, float b, float c)
{
        if (cnc->path == NULL) {
                r_warn("cnc_moveto: no path initiated");
                return -1;
        }
        if (cnc->v == 0.0f) {
                r_warn("cnc_moveto: current speed in zero");
                return -1;
        }
                
        action_t *action = new_move(x, y, z, a, b, c, cnc->v);
        if (action == NULL)
                return -1;
        path_append(cnc->path, action);
        return 0;
}

int cnc_delay(cnc_t *cnc, float seconds)
{        
        if (cnc->path == NULL) {
                r_warn("cnc_moveto: no path initiated");
                return -1;
        }
        action_t *action = new_delay(seconds);
        if (action == NULL)
                return -1;
        path_append(cnc->path, action);
        return 0;
}

int cnc_callback(cnc_t *cnc, cnc_callback_t cb, void *userdata)
{        
        if (cnc->path == NULL) {
                r_warn("cnc_moveto: no path initiated");
                return -1;
        }
        action_t *action = new_callback(cb, userdata);
        if (action == NULL)
                return -1;
        path_append(cnc->path, action);
        return 0;
}

int cnc_end_path(cnc_t *cnc)
{
        if (cnc->path == NULL) {
                r_warn("cnc_moveto: no path initiated");
                return -1;
        }
        return 0;
}

int cnc_run_path(cnc_t *cnc)
{
        if (cnc->path == NULL) {
                r_warn("cnc_moveto: no path initiated");
                return -1;
        }
        return 0;
}

int cnc_get_position(cnc_t *cnc, float *p)
{
        return controller_get_position(cnc->controller, p);
}

/**
 *
 */

int main(int argc, char **argv)
{
        controller_t *controller;
        cnc_t *cnc;

        r_log_init();
        r_log_set_app("oquah");
        
        controller = new_virtual_controller();
        if (controller == NULL) {
                r_err("Failed to create the virtual controller");
                return 1;
        }

        float vmax[] = { 1.0f, 1.0f, 0.02f, 0, 0, 0 };
        controller_set_vmax(controller, vmax);
        
        float amax[] = { 0.1f, 0.1f, 0.01f, 0, 0, 0 };
        controller_set_amax(controller, amax);
        
        cnc = new_cnc(controller);
        if (cnc == NULL) {
                r_err("Failed to create the CNC");
                return 1;
        }

        float xc = 0.4f;
        float yc = 0.4f;
        float r = 0.3f;
        int segments = 36;

        cnc_begin_path(cnc);
        
        for (int i = 0; i < segments; i++) {
                float angle = 0.0;
                float x = xc + cosf(i * M_PI / segments);
                float y = xc + sinf(i * M_PI / segments);
                cnc_moveto(cnc, x, y, 0, 0, 0, 0);
        }
        
        cnc_end_path(cnc);
        cnc_run_path(cnc);

        return 0;
}
