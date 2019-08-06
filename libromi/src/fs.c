#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
