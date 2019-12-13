#include <r.h>
#include "point.h"

point_t *new_point(float x, float y, float z)
{
        point_t *p = r_new(point_t);
        if (p == NULL)
                return NULL;
        p->x = x;
        p->y = y;
        p->z = z;
        return p;
}

void delete_point(point_t *p)
{
        if (p)
                r_delete(p);
}

void point_set(point_t *p, float x, float y, float z)
{
        p->x = x;
        p->y = y;
        p->z = z;
}
