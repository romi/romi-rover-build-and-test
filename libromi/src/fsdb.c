#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <rcom.h>

#include "romi.h"

/**************************************************************/

scan_t *fileset_get_scan(fileset_t *fileset);
database_t *scan_get_database(scan_t *scan);
int database_get_file_path(database_t *db, scan_t *scan,
                           fileset_t *fileset, file_t *file,
                           char *buffer, int len);
int database_store_scan(database_t *db, scan_t *scan);

/**************************************************************/

struct _file_t {
        fileset_t *fileset;
        char *id;
        char *name;
        double timestamp;
        vector_t position;
        file_t *next;
};

file_t *new_file(fileset_t *fileset, const char *id, double timestamp, vector_t position)
{
        file_t *file = new_obj(file_t);
        if (file == NULL) return NULL;
        memset(file, 0, sizeof(file_t));

        file->fileset = fileset;
        file->id = mem_strdup(id);
        file->name = NULL;
        file->timestamp = timestamp;
        file->position = position;
        file->next = NULL;
        
        if (file->id == NULL) {
                delete_obj(file);
                return NULL;
        }
        
        return file;
}

void delete_file( file_t *file)
{
        if (file) {
                if (file->id) mem_free(file->id);
                if (file->name) mem_free(file->name);
                delete_obj(file);
        }
}

fileset_t *file_get_fileset(file_t *file)
{
        return file->fileset;
}

int file_get_path(file_t *file, char *buffer, int len)
{
        fileset_t *fileset = file_get_fileset(file);
        scan_t *scan = fileset_get_scan(fileset);
        database_t *db = scan_get_database(scan);
        return database_get_file_path(db, scan, fileset, file, buffer, len);
}

int file_import_jpeg(file_t *file, const unsigned char *image, int len)
{
        char name[256];
        snprintf(name, 256, "%s.jpg", file->id);
        name[255] = 0;

        file->name = mem_strdup(name);
        if (file->name == NULL)
                return -1;

        char path[1024];
        file_get_path(file, path, 1024);
        
        int fd = open(path,
                      O_WRONLY | O_CREAT,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if (fd == -1) {
                log_warn("Failed to open '%s': %s.", path, strerror(errno));
                return -1;
        }
        
        ssize_t n, wrote = 0;
        while (wrote < len) {
                n = write(fd, image + wrote, len - wrote);
                if (n == -1) {
                        log_warn("Failed to write '%s': %s.", path, strerror(errno));
                        close(fd);
                        return -1;
                }
                wrote += n;
        }
        close(fd);
        return 0;
}

/**************************************************************/

struct _fileset_t {
        scan_t *scan;
        char *id;
        file_t *files;
        int num_files;
        fileset_t *next;
};

fileset_t *new_fileset(scan_t *scan, const char *id)
{
        fileset_t *fileset = new_obj(fileset_t);
        if (fileset == NULL) return NULL;

        fileset->scan = scan;
        fileset->id = mem_strdup(id);
        fileset->files = NULL;
        fileset->num_files = 0;
        fileset->next = NULL;
        
        if (fileset->id == NULL) {
                delete_obj(fileset);
                return NULL;
        }
        
        return fileset;
}

void delete_fileset(fileset_t *fileset)
{
        if (fileset) {
                file_t *l = fileset->files;
                while (l) {
                        file_t* f = l;
                        l = l->next;
                        delete_file(f);
                }
                if (fileset->id) mem_free(fileset->id);
                delete_obj(fileset);
        }
}

scan_t *fileset_get_scan(fileset_t *fileset)
{
        return fileset->scan;
}

file_t *fileset_new_file(fileset_t *fileset, double timestamp, vector_t position)
{
        char id[32];
        snprintf(id, 32, "%05d", fileset->num_files);
        id[31] = 0;
        
        file_t *file = new_file(fileset, id, timestamp, position);
        if (file == NULL) return NULL;
        file->next = fileset->files;
        fileset->files = file;
        fileset->num_files++;
        return file;
}

/**************************************************************/

struct _scan_t {
        database_t *db;
        char *id;
        fileset_t *filesets;
};

scan_t *new_scan(database_t *db, const char *id)
{
        scan_t *scan = new_obj(scan_t);
        if (scan == NULL) return NULL;

        scan->db = db;
        scan->id = mem_strdup(id);
        scan->filesets = NULL;
        
        if (scan->id == NULL) {
                delete_obj(scan);
                return NULL;
        }
        
        return scan;
}

void delete_scan(scan_t *scan)
{
        if (scan) {
                fileset_t *l = scan->filesets;
                while (l) {
                        fileset_t* f = l;
                        l = l->next;
                        delete_fileset(f);
                }
                if (scan->id) mem_free(scan->id);
                delete_obj(scan);
        }
}

database_t *scan_get_database(scan_t *scan)
{
        return scan->db;
}

int scan_add_fileset(scan_t *scan, fileset_t *fileset)
{
        fileset->next = scan->filesets;
        scan->filesets = fileset;
        return 0;
}

/**************************************************************/

struct _database_t {
        char *path;
};

database_t *new_database(const char *path)
{
        database_t *db = new_obj(database_t);
        if (db == NULL) return NULL;
        memset(db, 0, sizeof(database_t));

        db->path = mem_strdup(path);
        if (db->path == NULL) {
                delete_obj(db);
                return NULL;
        }
        
        return db;
}

void delete_database(database_t *db)
{
        if (db) {
                if (db->path) mem_free(db->path);
                delete_obj(db);
        }
}

scan_t *database_new_scan(database_t *db)
{
        char path[1024];
        char id[256];
        
        clock_datetime(id, sizeof(id), '-', '_', '-');
        
        snprintf(path, 1024, "%s/%s", db->path, id);
        path[1023] = 0;
	if (mkdir(path, 0777) == -1) {
                log_err("Failed to create directory '%s': %s.",
                        path, strerror(errno));
                return NULL;
	}

        log_info("new scan '%s' in directory '%s'", id, path);

        snprintf(path, 1024, "%s/%s/metadata", db->path, id);
        path[1023] = 0;
	if (mkdir(path, 0777) == -1) {
                log_err("Failed to create directory '%s': %s.",
                        path, strerror(errno));
                return NULL;
	}


        return new_scan(db, id);
}

fileset_t *database_new_fileset(database_t *db, scan_t *scan, const char *id)
{
        char path[1024];
        
        snprintf(path, 1024, "%s/%s/%s", db->path, scan->id, id);
        path[1023] = 0;
	if (mkdir(path, 0777) == -1) {
                log_err("Failed to create directory '%s': %s.",
                        path, strerror(errno));
                return NULL;
	}

        snprintf(path, 1024, "%s/%s/metadata/%s", db->path, scan->id, id);
        path[1023] = 0;
	if (mkdir(path, 0777) == -1) {
                log_err("Failed to create directory '%s': %s.",
                        path, strerror(errno));
                return NULL;
	}
        
        fileset_t *fileset = new_fileset(scan, id);
        scan_add_fileset(scan, fileset);

        return fileset;
}

int database_get_file_path(database_t *db,
                           scan_t *scan,
                           fileset_t *fileset,
                           file_t *file,
                           char *buffer, int len)
{
        snprintf(buffer, len, "%s/%s/%s/%s", db->path, scan->id, fileset->id, file->name);
        buffer[len-1] = 0;
        return 0;
}

int database_store_scan(database_t *db, scan_t *scan)
{
        char path[1024];

        snprintf(path, 1024, "%s/%s/files.json", db->path, scan->id);
        path[1023] = 0;

        log_info("Storing filesets to '%s'", path);
        
        FILE* fp = fopen(path, "w");
        if (fp == NULL) {
                log_warn("Failed to open file '%s'", path);
                return -1;
        }
        
        fprintf(fp, "{\"filesets\": [");

        fileset_t *filesets = scan->filesets;
        while (filesets) {
                fileset_t *fileset = filesets;

                fprintf(fp, "{\"id\": \"%s\", \"files\": [", fileset->id);

                file_t *files = fileset->files;
                while (files) {
                        file_t *file = files;
                        fprintf(fp, "{\"file\": \"%s\", \"id\": \"%s\"}", file->name, file->id);

                        {
                                // store metadata
                                snprintf(path, 1024, "%s/%s/metadata/%s/%s.json",
                                         db->path, scan->id, fileset->id, file->id);
                                path[1023] = 0;
                                FILE* fmeta = fopen(path, "w");
                                if (fmeta == NULL) {
                                        log_warn("Failed to open file '%s'", path);
                                        
                                } else {
                                        fprintf(fmeta, "{\"pose\": [%f,%f,%f,0,0], \"timestamp\": %f}",
                                                file->position.x, file->position.y, file->position.z,
                                                file->timestamp);
                                        fclose(fmeta);
                                }
                        }
                        
                        files = files->next;
                        if (files) fprintf(fp, ", ");
                } 
                fprintf(fp, "]}");

                filesets = filesets->next;
                if (filesets) fprintf(fp, ", ");
                
        }
        fprintf(fp, "]}");
        fclose(fp);

        return 0;
}
