#include <r.h>
#include "script.h"

typedef struct _script_t {
        double d;
        list_t *actions;
} script_t;

script_t *new_script(double d)
{
        script_t *script = r_new(script_t);
        if (script == NULL)
                return NULL;
        script->d = d;
        return script;
}

void delete_script(script_t *script)
{
        if (script) {
                for (list_t *l = script->actions; l; l = list_next(l)) {
                        action_t *n = list_get(l, action_t);
                        delete_action(n);
                }
                delete_list(script->actions);
                r_delete(script);
        }
}

void script_append(script_t *script, action_t *action)
{
        script->actions = list_append(script->actions, action);
}

list_t *script_actions(script_t *script)
{
        return script->actions;
}

double script_deviation(script_t *script)
{
        return script->d;
}
