#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "romi.h"

int is_absolute(const char *path)
{
        return path[0] == '/'; // FIXME!
}

int make_absolute_path(const char *path, char *buffer, int len)
{
        int r;
        
        if (is_absolute(path)) {
                r = snprintf(buffer, len, "%s", path);
        } else {
                char *wd = getcwd(NULL, 0);
                if (wd == NULL) return -1;
                r = snprintf(buffer, len, "%s/%s", wd, path);
                free(wd);
        }
        
        buffer[len-1] = 0;
        return r > len;
}

list_t *dir_list(const char *path)
{
        DIR *d;
        struct dirent *dir;
        list_t *list = NULL;
        
        d = opendir(path);
        if (d == NULL)
                return NULL;

        while ((dir = readdir(d)) != NULL) {
                if (strcmp(dir->d_name, ".") == 0
                    || strcmp(dir->d_name, "..") == 0)
                        continue;
                char *s = mem_strdup(dir->d_name);
                list = list_append(list, s);
        }
        closedir(d);
        return list;        
}

