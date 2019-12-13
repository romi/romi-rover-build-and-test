#ifndef _ROMI_POINT_H_
#define _ROMI_POINT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _point_t {
        float x, y, z;
} point_t;

point_t *new_point(float x, float y, float z);
void delete_point(point_t *p);
void point_set(point_t *p, float x, float y, float z);
        
#ifdef __cplusplus
}
#endif

#endif // _ROMI_POINT_H_
