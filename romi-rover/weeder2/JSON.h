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
#ifndef __ROMI_JSON_H
#define __ROMI_JSON_H

#include <exception>
#include <string>
#include <r.h>

namespace romi {

        class JSONError : public std::exception
        {
        protected:
                std::string _what;
        public:
                JSONError() : std::exception() {}
                
                virtual const char* what() const noexcept override { // TODO
                        return _what.c_str(); 
                }
        };

        class JSONTypeError : public JSONError
        {
        public:
                JSONTypeError(const char *expected) : JSONError() {
                        _what = "Invalid type. Expected JSON type ";
                        _what += expected;
                }
        };

        class JSONKeyError : public JSONError
        {
        public:
                JSONKeyError(const char *key) : JSONError() {
                        _what = "Invalid key: ";
                        _what += key;
                }
        };

        class JSONIndexError : public JSONError
        {
        public:
                JSONIndexError(int index) : JSONError() {
                        _what = "Index out of bounds: ";
                        _what += std::to_string(index);
                }
        };

        class JSONParseError : public JSONError
        {
        public:
                JSONParseError(const char *msg) : JSONError() {
                        _what  = msg;
                }
        };

        class JSON
        {
        protected:
                json_object_t _obj;
                
        public:

                static JSON load(const char *filename) {
                        int err;
                        char errmsg[128];
                        json_object_t obj = json_load(filename, &err, errmsg, 128);
                        if (err != 0)
                                throw JSONParseError(errmsg);
                        JSON json(obj);
                        json_unref(obj);
                        return json;
                }

                static JSON parse(const char *s) {
                        int err;
                        char errmsg[128];
                        json_object_t obj = json_parse_ext(s, &err, errmsg, 128);
                        if (err != 0)
                                throw JSONParseError(errmsg);
                        JSON json(obj);
                        json_unref(obj);
                        return json;
                }

                static JSON construct(const char *format, ...) {
                        va_list ap;
                        membuf_t *buffer = new_membuf();
        
                        va_start(ap, format);
                        membuf_vprintf(buffer, format, ap);
                        va_end(ap);

                        membuf_append_zero(buffer);
                        JSON obj = JSON::parse(membuf_data(buffer));

                        delete_membuf(buffer);
                        
                        return obj;
                }

                static JSON moveto(json_object_t obj) {
                        JSON temp(obj);
                        json_unref(obj);
                        return temp;
                }
                
                JSON() {
                        _obj = json_null();
                }
                
                JSON(json_object_t obj) {
                        _obj = obj;
                        json_ref(_obj);
                }
                
                JSON(JSON &json) {
                        _obj = json._obj;
                        json_ref(_obj);
                }
                
                virtual ~JSON() {
                        json_unref(_obj);
                }

                JSON &operator= (const JSON &rhs) {
                        json_object_t old = _obj;
                        _obj = rhs._obj;
                        json_ref(_obj);
                        json_unref(old);
                        return *this;
                }
                
                json_object_t ptr() {
                        return _obj;
                }
                
                bool has(const char *key) {
                        return (json_isobject(_obj) && json_object_has(_obj, key));
                }

                JSON get(const char *key) {
                        if (!json_isobject(_obj))
                                throw JSONTypeError("object");
                        if (!json_object_has(_obj, key))
                                throw JSONKeyError(key);
                        JSON retval(json_object_get(_obj, key));
                        return retval;
                }

                double num(const char *key) {
                        if (!json_isobject(_obj))
                                throw JSONTypeError("object");
                        if (!json_object_has(_obj, key))
                                throw JSONKeyError(key);
                        json_object_t value = json_object_get(_obj, key);
                        if (!json_isnumber(value))
                                throw JSONTypeError("number");
                        return json_number_value(value);
                }

                double num(const char *key, double default_value) {
                        double retval = default_value;
                        if (!json_isobject(_obj))
                                throw JSONTypeError("object");
                        if (json_object_has(_obj, key)) {
                                json_object_t value = json_object_get(_obj, key);
                                if (!json_isnumber(value))
                                        throw JSONTypeError("number");
                                retval = json_number_value(value);
                        }
                        return retval;
                }

                const char *str(const char *key) {
                        if (!json_isobject(_obj))
                                throw JSONTypeError("object");
                        if (!json_object_has(_obj, key))
                                throw JSONKeyError(key);
                        json_object_t value = json_object_get(_obj, key);
                        if (!json_isstring(value))
                                throw JSONTypeError("string");
                        return json_string_value(value);
                }

                JSON array(const char *key) {
                        if (!json_isobject(_obj))
                                throw JSONTypeError("object");
                        if (!json_object_has(_obj, key))
                                throw JSONKeyError(key);
                        json_object_t value = json_object_get(_obj, key);
                        if (!json_isarray(value))
                                throw JSONTypeError("array");
                        JSON json(value);
                        return json;
                }

                JSON get(int index) {
                        if (!json_isarray(_obj))
                                throw JSONTypeError("array");
                        if (index < 0 || index >= json_array_length(_obj))
                                throw JSONIndexError(index);
                        JSON retval = json_array_get(_obj, index);
                        return retval;
                }

                double num(int index) {
                        if (!json_isarray(_obj))
                                throw JSONTypeError("array");
                        if (index < 0 || index >= json_array_length(_obj))
                                throw JSONIndexError(index);
                        json_object_t value = json_array_get(_obj, index);
                        if (!json_isnumber(value))
                                throw JSONTypeError("number");
                        return json_number_value(value);
                }

                const char *str(int index) {
                        if (!json_isarray(_obj))
                                throw JSONTypeError("array");
                        if (index < 0 || index >= json_array_length(_obj))
                                throw JSONIndexError(index);
                        json_object_t value = json_array_get(_obj, index);
                        if (!json_isstring(value))
                                throw JSONTypeError("string");
                        return json_string_value(value);
                }

                JSON array(int index) {
                        if (!json_isarray(_obj))
                                throw JSONTypeError("array");
                        if (index < 0 || index >= json_array_length(_obj))
                                throw JSONIndexError(index);
                        json_object_t value = json_array_get(_obj, index);
                        if (!json_isarray(value))
                                throw JSONTypeError("array");
                        JSON json(value);
                        return json;
                }

                int length() {
                        if (!json_isarray(_obj))
                                throw JSONTypeError("array");
                        return json_array_length(_obj);
                }

                double num() {
                        if (!json_isnumber(_obj))
                                throw JSONTypeError("number");
                        return json_number_value(_obj);
                }

                const char *str() {
                        if (!json_isstring(_obj))
                                throw JSONTypeError("string");
                        return json_string_value(_obj);
                }
                
                int32 foreach(json_iterator_t func, void* data) {
                        return json_object_foreach(_obj, func, data);
                }

        };
}

#endif // __ROMI_JSON_H
