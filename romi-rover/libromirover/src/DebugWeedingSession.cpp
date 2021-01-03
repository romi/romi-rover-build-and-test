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

#include "DebugWeedingSession.h"
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

namespace romi {

        IFolder &DebugWeedingSession::start_new_folder()
        {
                std::string path;
                make_path(path);
                if (make_folder(path)) {
                        _folder.set_directory(path);
                        return _folder;
                } else {
                        return _error_folder;
                }
        }

        void DebugWeedingSession::make_path(std::string &path)
        {
                static char timestamp[128];
                path = _directory;
                path += "/";
                path += _basename;
                path += "-";
                path += clock_datetime_compact(timestamp, sizeof(timestamp));
                path += "-";
                path += std::to_string(_count++);
        }

        bool DebugWeedingSession::make_folder(std::string &path)
        {
                bool success = false;
                struct stat st;
                memset(&st, 0, sizeof(st));
                if (stat(path.c_str(), &st) == -1) {
                        if (mkdir(path.c_str(), 0770) == 0)
                                success = true;
                } else {
                        success = true;
                }
                return success;
        }
        
        std::string DebugWeedingFolder::make_filename(const char* name,
                                                       const char *extension)
        {
                std::string filename;
                if (_directory.length()) {
                        filename = _directory;
                        filename += "/";
                }
                filename += name;
                filename += ".";
                filename += extension;
                return filename;
        }

        void DebugWeedingFolder::store(const char* name, Image &image)
        {
                store(name, image.ptr());
        }

        void DebugWeedingFolder::store_jpg(const char* name, Image &image)
        {
                store_jpg(name, image.ptr());
        }
        
        void DebugWeedingFolder::store_png(const char* name, Image &image)
        {
                store_png(name, image.ptr());
        }

        void DebugWeedingFolder::store(const char* name, image_t *image)
        {
                if (image)
                        store_jpg(name, image);
        }
        
        void DebugWeedingFolder::store_jpg(const char* name, image_t *image)
        {
                if (image) {
                        std::string filename = make_filename(name, "jpg");
                        image_store(image, filename.c_str(), "jpg");
                }
        }
        
        void DebugWeedingFolder::store_png(const char* name, image_t *image)
        {
                if (image) {
                        std::string filename = make_filename(name, "png");
                        image_store(image, filename.c_str(), "png");
                }
        }

        void DebugWeedingFolder::store_svg(const char* name, const char *body, int len)
        {
                std::string filename = make_filename(name, "svg");                
                std::ofstream file;
                file.open(filename);
                file.write(body, len);
                file.close();
        }

        void DebugWeedingFolder::store_txt(const char* name, const char *body, int len)
        {
                std::string filename = make_filename(name, "txt");                
                std::ofstream file;
                file.open(filename);
                file.write(body, len);
                file.close();
        }

        void DebugWeedingFolder::open_dump()
        {
                close_dump();
                _dump_fd = ::open("dump.out",
                                  O_WRONLY | O_CREAT | O_TRUNC,
                                  S_IRUSR | S_IWUSR);
                if (_dump_fd == -1)
                        r_err("DebugWeedingFolder::open_dump: failed to open dump.out");
        }

        void DebugWeedingFolder::dump_int(int32_t value)
        {
                ssize_t n = write(_dump_fd, (void *) &value, 4);
                if (n != 4)
                        r_err("dump: write error");
        }

        void DebugWeedingFolder::dump_text(const char *s, ssize_t len)
        {
                ssize_t n = write(_dump_fd, s, len);
                if (n != len)
                        r_err("dump: write error");
        }

        void DebugWeedingFolder::dump_double(double value)
        {
                ssize_t n = write(_dump_fd, (void *) &value, sizeof(double));
                if (n != sizeof(double))
                        r_err("dump: write error");
        }
        
        void DebugWeedingFolder::dump(const char *name, int32_t rows,
                                       int32_t cols, float *values)
        {
                if (_dump_fd != -1) {
                        int32_t len = strlen(name);
                        if (len > 31)
                                len = 31;
                        dump_int(len);
                        dump_text(name, len);
                        dump_int(rows);
                        dump_int(cols);
        
                        int index = 0;
                        for (int row = 0; row < rows; row++) { 
                                for (int col = 0; col < cols; col++) {
                                        dump_double(values[index]);
                                        index++;
                                }
                        }
                }
        }
        
        void DebugWeedingFolder::dump(const char *name, int32_t rows,
                                       int32_t cols, double *values)
        {
                if (_dump_fd != -1) {
                        int32_t len = strlen(name);
                        if (len > 31)
                                len = 31;
                        dump_int(len);
                        dump_text(name, len);
                        dump_int(rows);
                        dump_int(cols);
        
                        int index = 0;
                        for (int row = 0; row < rows; row++) { 
                                for (int col = 0; col < cols; col++) {
                                        dump_double(values[index]);
                                        index++;
                                }
                        }
                }
        }
        
        void DebugWeedingFolder::dump_interleave(const char *name, int32_t size, 
                                                  float *a, float *b)
        {
                if (_dump_fd != -1) {
                        int32_t len = strlen(name);
                        if (len > 31)
                                len = 31;
                        dump_int(len);
                        dump_text(name, len);
                        dump_int(size);
                        dump_int(2);
        
                        for (int i = 0; i < size; i++) { 
                                dump_double(a[i]);
                                dump_double(b[i]);
                        }
                }
        }
        
        void DebugWeedingFolder::dump_interleave(const char *name, int32_t size,
                                                  double *a, double *b)
        {
                if (_dump_fd != -1) {
                        int32_t len = strlen(name);
                        if (len > 31)
                                len = 31;
                        dump_int(len);
                        dump_text(name, len);
                        dump_int(size);
                        dump_int(2);
                        
                        for (int i = 0; i < size; i++) { 
                                dump_double(a[i]);
                                dump_double(b[i]);
                        }
                }
        }
        
        void DebugWeedingFolder::close_dump()
        {
                if (_dump_fd != -1) {
                        ::close(_dump_fd);
                        _dump_fd = -1;
                }
        }

        void DebugWeedingFolder::print_path(float *x, float *y, int len, int n)
        {
                char name[64];
                if (n >= 0)
                        snprintf(name, 64, "path-%05d", n);
                else
                        snprintf(name, 64, "path");
                        
                std::string filename = make_filename(name, "txt");
                std::ofstream file;
                file.open(filename);
                for (int i = 0; i < len; i++)
                        file << x[i] << "\t" << y[i] << std::endl;
                file.close();
        }
        
        void DebugWeedingFolder::print_path(double *x, double *y, int len, int n)
        {
                char name[64];
                if (n >= 0)
                        snprintf(name, 64, "path-%05d", n);
                else
                        snprintf(name, 64, "path");
                        
                std::string filename = make_filename(name, "txt");
                std::ofstream file;
                file.open(filename);
                for (int i = 0; i < len; i++)
                        file << x[i] << "\t" << y[i] << std::endl;
                file.close();
        }
        
        void DebugWeedingFolder::print_path(Path &path, int n)
        {
                char name[64];
                if (n >= 0)
                        snprintf(name, 64, "path-%05d", n);
                else
                        snprintf(name, 64, "path");
                        
                std::string filename = make_filename(name, "txt");
                std::ofstream file;
                file.open(filename);
                
                Path::iterator ptr; 
                for (ptr = path.begin(); ptr < path.end(); ptr++)  {
                        v3 p = *ptr;
                        file << p.x() << "\t" << p.y() << std::endl;
                }
                file.close();
        }
        

}
