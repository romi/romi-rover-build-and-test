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
#include <r.h>
#include "GetOpt.h"

using namespace std;

namespace romi {
        
        GetOpt::GetOpt(Option *list, size_t len)
                : _options(list), _length(len)
        {
                generate_long_options();
                generate_descriptions();
        }

        void GetOpt::generate_long_options()
        {
                for (size_t i = 0; i < _length; i++) {
                        append_long_option(_options[i]);
                }
                append_zero_option();
        }
        
        void GetOpt::append_long_option(Option& option)
        {
                struct option long_option = { option.name, no_argument, 0, 0 };
                if (option.requires_value)
                        long_option.has_arg = required_argument;
                _long_options.push_back(long_option);
        }
        
        void GetOpt::append_zero_option()
        {
                struct option long_option = { 0, 0, 0, 0 };
                _long_options.push_back(long_option);
        }
        
        void GetOpt::generate_descriptions()
        {
                for (size_t i = 0; i < _length; i++)
                        _descriptions.push_back(_options[i].description);
        }
        
        void GetOpt::parse(int argc, char **argv)
        {
                int option_index;
                optind = 1;
                
                while (1) {
                        int c = getopt_long(argc, argv, "", &_long_options[0],
                                            &option_index);
                        if (c == -1)
                                break;
                
                        if (c == '?')
                                r_warn("Unrecognized option");
                        else
                                set_option(option_index, optarg);
                }
        }

        void GetOpt::set_option(int index, const char *value)
        {
                const char *name = _long_options[index].name;
                if (_long_options[index].has_arg == required_argument)
                        _values[name] = value;
                else
                        _flags[name] = true;
        }
        
        bool GetOpt::get_flag(const char *name)
        {
                return _flags.find(name) != _flags.end();
        }

        const char *GetOpt::get_value(const char *name)
        {
                const char *retval;
                map<string, string>::iterator index = _values.find(name);
                if (index != _values.end())
                        retval = index->second.c_str();
                else
                        retval = get_default_value(name);
                return retval;
        }

        const char *GetOpt::get_default_value(const char *name)
        {
                const char *retval = 0;
                for (size_t i = 0; i < _length; i++) {
                        if (rstreq(_options[i].name, name)) {
                                retval = _options[i].default_value;
                                break;
                        }
                }
                return retval;
        }
        
        bool GetOpt::is_help_requested()
        {
                return get_flag("help");
        }
        
        void GetOpt::print_usage()
        {
                printf("Usage: romi-rover [options]\n");
                printf("Options:\n");
                for (size_t i = 0; i < _length; i++) {
                        printf("--%s", _options[i].name);
                        if (_options[i].requires_value)
                                printf(" value");
                        printf(": %s", _options[i].description);
                        if (_options[i].requires_value
                            && _options[i].default_value)
                                printf(" (default: '%s')", _options[i].default_value);
                        printf("\n");
                }
        }
}

