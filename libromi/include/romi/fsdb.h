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

/*
  Database
 */
database_t *new_database(const char *path);
void delete_database(database_t *db);
int database_load(database_t *db);
int database_unload(database_t *db);

scan_t *database_new_scan(database_t *db);

int database_count_scans(database_t *db);
scan_t *database_get_scan_at(database_t *db, int i);
scan_t *database_get_scan(database_t *db, const char *id);

void database_print(database_t *db);

/*
  Scan
 */
fileset_t *scan_new_fileset(scan_t *scan, const char *id);
int scan_store(scan_t *scan);

const char *scan_id(scan_t *scan);
int scan_count_filesets(scan_t *scan);
fileset_t *scan_get_fileset_at(scan_t *scan, int i);
fileset_t *scan_get_fileset(scan_t *scan, const char *id);
database_t *scan_get_database(scan_t *scan);

/*
  Fileset
 */
file_t *fileset_new_file(fileset_t *fileset);

const char *fileset_id(fileset_t *fileset);
int fileset_count_files(fileset_t *fileset);
file_t *fileset_get_file_at(fileset_t *fileset, int i);
file_t *fileset_get_file(fileset_t *fileset, const char *id);
file_t *fileset_get_file_by_name(fileset_t *fileset, const char *name);
scan_t *fileset_get_scan(fileset_t *fileset);

/*
  File
 */
void file_set_timestamp(file_t *file, double timestamp);
double file_get_timestamp(file_t *file);

void file_set_position(file_t *file, vector_t position);
vector_t file_get_position(file_t *file);

json_object_t file_get_metadata(file_t *file);
        
int file_import_jpeg(file_t *file, const unsigned char *image, int len);

const char *file_id(file_t *file);
const char *file_name(file_t *file);
const char *file_mimetype(file_t *file);
int file_path(file_t *file, char *buffer, int len);
fileset_t *file_get_fileset(file_t *file);

#ifdef __cplusplus
}
#endif

#endif  /* _ROMI_FSDB_H_ */

