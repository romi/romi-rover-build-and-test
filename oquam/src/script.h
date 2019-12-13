#include <r.h>
#include "action.h"

#ifndef _OQUAM_SCRIPT_H_
#define _OQUAM_SCRIPT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _script_t script_t;

script_t *new_script(double d);
void delete_script(script_t *script);
void script_append(script_t *script, action_t *action);
list_t *script_actions(script_t *script);
double script_deviation(script_t *script);

#ifdef __cplusplus
}
#endif

#endif // _OQUAM_SCRIPT_H_
