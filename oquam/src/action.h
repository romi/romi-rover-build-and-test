
#ifndef _OQUAM_ACTION_H_
#define _OQUAM_ACTION_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
        ACTION_WAIT = 0,
        ACTION_MOVE,
        ACTION_DELAY,
        ACTION_TRIGGER
};

typedef struct _action_t action_t;

struct _action_t {
        unsigned char type;
        // move
        double p[3];
        double v;
        // delay
        double delay;
        // trigger
        void *callback;
        void *userdata;
        int arg;
};

action_t *new_action(int type);
action_t *action_clone(action_t *a);
action_t *new_wait();
action_t *new_delay(double delay);
action_t *new_move(double x, double y, double z, double v);
action_t *new_trigger(void *callback, void *userdata, int arg);
void delete_action(action_t *action);

#ifdef __cplusplus
}
#endif

#endif // _OQUAM_ACTION_H_
