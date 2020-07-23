#ifndef __PARSER_H
#define __PARSER_H

#include <stdint.h>

enum parser_error_t {
        parser_error_none = 0,
        parser_unexpected_char = 1,
        parser_vector_too_long = 2,
        parser_value_out_of_range = 3
};

class Parser
{
protected:
        char _state;
        char _error;
        char _opcode;
        int16_t _value[32];
        int _length;
        int32_t _tmpval;
        int16_t _sign;
        const char* _opcodesWithValue;
        const char* _opcodesWithoutValue;

        void set_error(char what);

public:
        
        Parser(const char* opcodesWithValue,
               const char* opcodesWithoutValue) {
                _opcodesWithValue = opcodesWithValue;
                _opcodesWithoutValue = opcodesWithoutValue;
                _state = 0;
        }

        int16_t value(int index = 0) {
                return (index < _length)? _value[index] : 0;
        }

        int length() {
                return _length;
        }

        char opcode() {
                return _opcode;
        }
        
        char error() {
                return _error;
        }
        
        bool process(char c);
};

#endif
