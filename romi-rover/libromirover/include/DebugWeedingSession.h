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

#ifndef __ROMI_DEBUG_WEEDING_SESSION_H
#define __ROMI_DEBUG_WEEDING_SESSION_H

#include <string>
#include "IFolder.h"
#include "IFileCabinet.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace romi {

        class DebugWeedingFolder : public IFolder
        {
        protected:
                std::string _directory;
                int _dump_fd;
                
                std::string make_filename(const char* name, const char *extension);
                void dump_int(int32_t value);
                void dump_text(const char *s, ssize_t len);
                void dump_double(double value);

        public:
                DebugWeedingFolder() : _dump_fd(-1) {};
                virtual ~DebugWeedingFolder() override = default;
                
                void store(const char* name, Image &image) override;
                void store_jpg(const char* name, Image &image);
                void store_png(const char* name, Image &image);
                
                void store(const char* name, image_t *image) override;
                void store_jpg(const char* name, image_t *image) override;
                void store_png(const char* name, image_t *image) override;
                void store_svg(const char* name, const char *body, int len) override;
                void store_txt(const char* name, const char *body, int len) override;

                void open_dump() override;
                void dump(const char *name, int32_t rows,
                          int32_t cols, float *values) override;
                void dump(const char *name, int32_t rows,
                          int32_t cols, double *values) override;
                void dump_interleave(const char *name, int32_t size, 
                                     float *a, float *b) override;
                void dump_interleave(const char *name, int32_t size,
                                     double *a, double *b) override;
                void close_dump() override;
                
                void print_path(float *x, float *y, int len, int n = -1) override;
                void print_path(double *x, double *y, int len, int n = -1) override;
                void print_path(Path &path, int n = -1) override;
                
                void set_directory(std::string &path) {
                        _directory = path;
                }
        };
        
        class ErrorFolder : public IFolder
        {
        protected:
                void print_error_message(const char *name = 0) {
                        r_warn("Failed to create the directory. Can't store %s",
                               name? name : "the data");
                }

        public:
                virtual ~ErrorFolder() override = default;

                
                void store(const char* name, Image &image) override {
                        print_error_message(name);
                }
                void store_jpg(const char* name, Image &image) {
                        print_error_message(name);
                }
                void store_png(const char* name, Image &image) {
                        print_error_message(name);
                }
                void store(const char* name, image_t *image) override {
                        print_error_message(name);
                }
                void store_jpg(const char* name, image_t *image) override {
                        print_error_message(name);
                }
                void store_png(const char* name, image_t *image) override {
                        print_error_message(name);
                }
                void store_svg(const char* name, const char *body, int len) override {
                        print_error_message(name);
                }
                void store_txt(const char* name, const char *body, int len) override {
                        print_error_message(name);
                }
                void open_dump() override {
                        print_error_message("the dump");
                }
                void dump(const char *name, int32_t rows,
                          int32_t cols, float *values) override {
                        print_error_message(name);
                }
                void dump(const char *name, int32_t rows,
                          int32_t cols, double *values) override {
                        print_error_message(name);
                }
                void dump_interleave(const char *name, int32_t size, 
                                     float *a, float *b) override {
                        print_error_message(name);
                }
                void dump_interleave(const char *name, int32_t size,
                                     double *a, double *b) override {
                        print_error_message(name);
                }
                void close_dump() override {}
                void print_path(float *x, float *y, int len, int n = -1) override {
                        print_error_message();
                }
                void print_path(double *x, double *y, int len, int n = -1) override {
                        print_error_message();
                }
                void print_path(Path &path, int n = -1) override {
                        print_error_message();
                }
        };
        

        class DebugWeedingSession : public IFileCabinet
        {
        protected:
                std::string _directory;
                std::string _basename;
                DebugWeedingFolder _folder;
                ErrorFolder _error_folder; // FIXME
                
                bool make_folder(std::string &path) {
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

                void make_path(std::string &path) {
                        static char timestamp[128];
                        path = _directory;
                        path += "/";
                        path += _basename;
                        path += "-";
                        path += clock_datetime_compact(timestamp, sizeof(timestamp));
                }

        public:
                
                DebugWeedingSession(const char *basedir,
                                    const char *basename)
                        : _directory(basedir), _basename(basename) {
                        start_new_folder();
                }
                
                virtual ~DebugWeedingSession() override = default;
                
                IFolder &start_new_folder() override {
                        std::string path;
                        make_path(path);
                        if (make_folder(path)) {
                                _folder.set_directory(path);
                                return _folder;
                        } else {
                                return _error_folder;
                        }
                }
        };
        
}

#endif // __ROMI_DEBUG_WEEDING_SESSION_H
