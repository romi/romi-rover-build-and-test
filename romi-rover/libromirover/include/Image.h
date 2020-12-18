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

#ifndef __ROMI_IMAGE_H
#define __ROMI_IMAGE_H

#include <stdexcept>
#include "image_impl.h"

namespace romi {
        
        class Image
        {
        protected:
                image_t *_image;
                
        public:
                Image() : _image(0) {}
                
                Image(int type, int width, int height) {
                        _image = new_image(type, width, height);
                }

                Image(const unsigned char *data, int len) {
                        _image = image_load_from_mem(data, len);
                        if (_image == 0)
                                throw std::runtime_error("Failed to load the image");
                }
                
                virtual ~Image() {
                        if (_image) 
                                delete_image(_image);
                }

                Image &operator= (const Image &rhs) {
                        if (_image) 
                                delete_image(_image);
                        _image = image_clone(rhs._image);
                        return *this;
                }

                int type() {
                        return _image? _image->type : -1;
                }

                float *data() {
                        return _image? _image->data : NULL;
                }

                int width() {
                        return _image? _image->width : 0;
                }

                int height() {
                        return _image? _image->height : 0;
                }
                
                void moveto(image_t *image) {
                        if (_image) 
                                delete_image(_image);
                        _image = image;
                }

                void load_from_file(const char *filename) {
                        if (_image) 
                                delete_image(_image);
                        _image = image_load(filename);
                        if (_image == 0)
                                throw std::runtime_error("Failed to load the image");
                }

                void load_from_mem(const unsigned char *data, int len) {
                        if (_image) 
                                delete_image(_image);
                        _image = image_load_from_mem(data, len);
                        if (_image == 0)
                                throw std::runtime_error("Failed to load the image");
                }

                bool import_rgb(uint8_t* rgb, int width, int height) {
                        bool success = false;
                        if (_image == 0) {
                                _image = convert_to_image(rgb, width, height);
                                success = (_image != 0);
                        } else if (width != image_width(_image)
                                   || height != image_height(_image)
                                   || IMAGE_RGB != _image->type) {
                                delete_image(_image);
                                _image = convert_to_image(rgb, width, height);
                                success = (_image != 0);
                        } else {
                                image_import(_image, rgb);
                                success = true;
                        }
                        return success;
                }

                bool to_jpeg(membuf_t *out) {
                        bool success = false;
                        if (_image) 
                                success = (image_store_to_mem(_image, out, "jpg") == 0);
                        return success;
                }
                
                void crop(int x0, int y0, int width, int height, Image &out) {
                        image_t *cropped = FIXME_image_crop(_image, x0, y0, width, height);
                        if (cropped == 0)
                                throw std::runtime_error("Failed to crop the image");
                        out.moveto(cropped);
                }

                void scale(int n, Image &out) {
                        image_t *scaled = FIXME_image_scale(_image, n);
                        out.moveto(scaled);
                }
                
                // FIXME: this method is temporary
                image_t *ptr() {
                        return _image;
                }

        };
}

#endif // __ROMI_IMAGE_H
