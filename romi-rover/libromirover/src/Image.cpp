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

#include <stdexcept>
#include <r.h>
#include "Image.h"

namespace romi {

        static size_t channels_per_type(Image::ImageType type)
        {
                size_t r = -1;
                switch (type) {
                case Image::BW:
                        r = 1;
                        break;
                case Image::RGB:
                        r = 3;
                        break;
                }
                return r;
        }
        
        Image::Image() : _width(0), _height(0), _type(RGB), _channels(3), _data(0)
        {
        }

        Image::Image(ImageType type, size_t width, size_t height)
                : _width(0), _height(0), _data(0)
        { 
                do_init(type, width, height);
        }
        
        Image::Image(ImageType type, const uint8_t *data, size_t width, size_t height)
                : _width(0), _height(0), _data(0)
        {
                do_init(type, width, height);
                import_data(data);
        }
        
        Image::~Image()
        {
                free_data();
        }

        void Image::do_init(ImageType type, size_t width, size_t height)
        {
                _type = type;
                _channels = channels_per_type(type);
                _width = width;
                _height = height;
                resize_data();
        }

        void Image::init(ImageType type, size_t width, size_t height)
        {
                if (_type != type || _width != width || _height != height) {
                        do_init(type, width, height);
                }
        }

        void Image::import(ImageType type, const uint8_t *data,
                           size_t width, size_t height)
        {
                init(type, width, height);
                import_data(data);
        }

        void Image::import_data(const uint8_t *data)
        {
                size_t len = length();
                float *p = _data;
                for (size_t i = 0; i < len; i++)
                        *p++ = (float) *data++ / 255.0f;
        }

        void Image::resize_data()
        {
                free_data();
                alloc_data();
        }

        void Image::free_data()
        {
                if (_data) {
                        r_free(_data);
                        _data = 0;
                }
        }

        void Image::alloc_data()
        {
                if (_width * _height > 0) {
                        _data = (float *) r_alloc(byte_length());
                        memset(_data, 0, byte_length());
                }
        }
        
        void Image::fill(size_t channel, float color)
        {
                size_t stride = _channels;
                size_t len = length();
                for (size_t i = channel; i < len; i += stride)
                        _data[i] = color;
        }
        
        void Image::crop(size_t x, size_t y, size_t width, size_t height, Image &out)
        {
                if (x >= _width) {
                        width = 0;
                } else if (x + width > _width) {
                        width = _width - x;
                }
                
                if (y >= _height) {
                        height = 0;
                } else if (y + height > _height) {
                        height = _height - y;
                }
                
                out.init(_type, width, height);
                
                for (size_t line = 0; line < height; line++) {
                        size_t len = width * _channels * sizeof(float);
                        size_t crop_offset = out.offset(0, 0, line);
                        size_t im_offset = offset(0, x, y + line);
                        memcpy(&out._data[crop_offset], &_data[im_offset], len); 
                }
        }

        void Image::scale(size_t n, Image &out)
        {
                if (n == 0)
                        n = 1;
                
                size_t width = (_width + n - 1) / n;
                size_t height = (_height + n - 1) / n;
                
                out.init(_type, width, height);

                for (size_t y = 0; y < height; y++) {
                        for (size_t x = 0; x < width; x++) {
                                size_t i0 = offset(0, n * x, n * y);
                                size_t i1 = out.offset(0, x, y);
                                for (size_t c = 0; c < _channels; c++) {
                                        out._data[i1 + c] = _data[i0 + c];
                                }
                        }
                }
        }

        size_t Image::length()
        {
                return _width * _height * _channels;
        }
        
        size_t Image::byte_length()
        {
                return length() * sizeof(float);
        }

        void Image::copy_to(Image &to)
        {
                to.init(_type, _width, _height);
                memcpy(to._data, _data, byte_length());
        }
}
