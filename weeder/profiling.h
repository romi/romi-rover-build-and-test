#ifndef _ROMI_PROFILING_H_
#define _ROMI_PROFILING_H_

#include <r.h>
#include <romi.h>
#include "weeding.h"

#ifdef __cplusplus
extern "C" {
#endif

fileset_t *create_fileset(scan_t *session, const char *fileset_id);

int store_workspace(fileset_t *fileset, workspace_t *workspace);
file_t *create_file(fileset_t *fileset, const char *name);
void store_jpg(fileset_t *fileset, const char *name, image_t *image);
void store_png(fileset_t *fileset, const char *name, image_t *image);
void store_text(fileset_t *fileset, const char *name, membuf_t *text);

        
#ifdef __cplusplus
}
#endif

#endif // _ROMI_PROFILING_H_
