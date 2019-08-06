#ifndef _ROMI_FSDB_H_
#define _ROMI_FSDB_H_

#include "romi/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _file_t file_t;
typedef struct _fileset_t fileset_t;
typedef struct _scan_t scan_t;
typedef struct _database_t database_t;

database_t *new_database(const char *path);
void delete_database(database_t *db);

scan_t *database_new_scan(database_t *db);
int database_store_scan(database_t *db, scan_t *scan);
void delete_scan(scan_t *scan);

fileset_t *database_new_fileset(database_t *db, scan_t *scan, const char *id);

file_t *fileset_new_file(fileset_t *fileset, double timestamp, vector_t position);
int file_import_jpeg(file_t *file, const unsigned char *image, int len);
        
#ifdef __cplusplus
}
#endif

#endif  /* _ROMI_FSDB_H_ */

