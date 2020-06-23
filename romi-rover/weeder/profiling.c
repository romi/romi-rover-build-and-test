/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
#include "profiling.h"

fileset_t *create_fileset(scan_t *session, const char *fileset_id)
{
        if (session == NULL) {
                r_debug("create_fileset: session is null");
                return NULL;
        }
        
        fileset_t *fileset = NULL;
        const char *s = fileset_id;
        char id[64];

        if (fileset_id == NULL) {
                int num = 0;
                while (num < 100000) {
                        rprintf(id, sizeof(id), "weeding-%05d", num);
                        fileset = scan_get_fileset(session, id);
                        if (fileset == NULL)
                                break;
                        num++;
                }
                s = id;
        }

        r_info("Storing weeding files in fileset '%s'", s);

        fileset = scan_get_fileset(session, s);

        if (fileset == NULL)
                fileset = scan_new_fileset(session, s);
        if (fileset == NULL)
                return NULL;
        
        return fileset;
}

void store_jpg(fileset_t *fileset, const char *name, image_t *image)
{
        if (fileset == NULL)
                return;
        fileset_store_image(fileset, name, image, "jpg");
}

void store_png(fileset_t *fileset, const char *name, image_t *image)
{
        if (fileset == NULL)
                return;
        fileset_store_image(fileset, name, image, "png");
}

file_t *create_file(fileset_t *fileset, const char *name)
{
        if (fileset == NULL)
                return NULL;
        file_t *file = fileset_new_file(fileset);
        if (file == NULL)
                return NULL;
        file_set_timestamp(file, clock_time());
        file_set_metadata_str(file, "name", name);
        return file;
}

void store_text(fileset_t *fileset, const char *name, membuf_t *text)
{
        if (fileset == NULL)
                return;
        
        file_t *file = create_file(fileset, name);
        if (file == NULL)
                return;
        
        file_import_data(file, membuf_data(text), membuf_len(text),
                         "text/plain", "txt");
}
