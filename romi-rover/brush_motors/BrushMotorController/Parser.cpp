#include <string.h>
#include "Parser.h"

enum parser_state_t {
        wait_opcode = 0,
        wait_bracket_or_minus_or_digit = 1,
        wait_minus_or_digit_scalar = 2,
        wait_minus_or_digit_vector = 3,
        wait_digit_scalar = 4,
        wait_digit_vector = 5,
        wait_digits_or_cr = 6,
        wait_digits_or_comma_or_bracket = 7,
        wait_carriage_return = 8,
        wait_opcode_or_linefeed = 9
};

#define MINUS(_c)            ((_c) == '-')
#define DIGIT(_c)            ((_c) >= '0' && (_c) <= '9')
#define OPENBRACKET(_c)      ((_c) == '[')
#define CLOSEBRACKET(_c)     ((_c) == ']')
#define LINEFEED(_c)         ((_c) == '\n')
#define CARRIAGE_RETURN(_c)  ((_c) == '\r')
#define COMMA(_c)            ((_c) == ',')
#define VALUE(_c)            ((int)((_c) - '0'))
#define OPCODE_W_VALUE(_c)   (strchr(_opcodesWithValue, (_c)) != NULL)
#define OPCODE_WO_VALUE(_c)  (strchr(_opcodesWithoutValue, (_c)) != NULL)


void Parser::set_error(char what)
{
        _state = wait_opcode;
        _error = what;
        _length = 0;
        _opcode = 0;
}

bool Parser::process(char c)
{
        bool r = false;
        
        switch(_state) {
        case wait_opcode:
                if (OPCODE_W_VALUE(c)) {
                        _opcode = c;
                        _length = 0;
                        _state = wait_bracket_or_minus_or_digit;
                        _error = parser_error_none;
                } else if (OPCODE_WO_VALUE(c)) {
                        _opcode = c;
                        _length = 0;
                        _state = wait_carriage_return;
                        _error = parser_error_none;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
                
        case wait_opcode_or_linefeed:
                if (LINEFEED(c)) {
                        // The command was terminated by \r\n
                        _state = wait_opcode; 
                } else {
                        // The command was terminated by \r. The new
                        // character belongs to the next command. Call
                        // process again with the proper state.
                        _state = wait_opcode;
                        process(c);
                }
                break;
                
        case wait_carriage_return:
                if (CARRIAGE_RETURN(c)) {
                        // The command may be terminated by \n, or by \r\n
                        _state = wait_opcode_or_linefeed;
                        r = true; 
                } else if (LINEFEED(c)) {
                        _state = wait_opcode;
                        r = true; // The command was terminated by \n
                } else {
                        set_error(parser_unexpected_char);
                }
                break;

        case wait_bracket_or_minus_or_digit:
                if (OPENBRACKET(c)) {
                        _tmpval = 0;
                        _sign = -1;
                        _state = wait_minus_or_digit_vector;
              } else if (MINUS(c)) {
                        _tmpval = 0;
                        _sign = -1;
                        _state = wait_digit_scalar;
                } else if (DIGIT(c)) {
                        _tmpval = VALUE(c);
                        _sign = 1;
                        _state = wait_digits_or_cr;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
        case wait_minus_or_digit_scalar:
                if (MINUS(c)) {
                        _tmpval = 0;
                        _sign = -1;
                        _state = wait_digit_scalar;
                } else if (DIGIT(c)) {
                        _tmpval = VALUE(c);
                        _sign = 1;
                        _state = wait_digits_or_cr;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
        case wait_digit_scalar:
                if (DIGIT(c)) {
                        _tmpval = _sign * VALUE(c);
                        _state = wait_digits_or_cr;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
        case wait_digits_or_cr:
                if (DIGIT(c)) {
                        _tmpval = 10 * _tmpval + _sign * VALUE(c);
                        _state = wait_digits_or_cr;
                        if (_tmpval > 32767 || _tmpval < -32768)
                                set_error(parser_value_out_of_range);
                } else if (CARRIAGE_RETURN(c)) {
                        _value[0] = _tmpval;
                        _length = 1;
                        _state = wait_opcode_or_linefeed;
                        r = true;
                } else if (LINEFEED(c)) {
                        _value[0] = _tmpval;
                        _length = 1;
                        _state = wait_opcode;
                        r = true;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
        case wait_minus_or_digit_vector:
                if (MINUS(c)) {
                        _tmpval = 0;
                        _sign = -1;
                        _state = wait_digit_vector;
                } else if (DIGIT(c)) {
                        _tmpval = VALUE(c);
                        _sign = 1;
                        _state = wait_digits_or_comma_or_bracket;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
        case wait_digit_vector:
                if (DIGIT(c)) {
                        _tmpval = _sign * VALUE(c);
                        _state = wait_digits_or_comma_or_bracket;
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
        case wait_digits_or_comma_or_bracket:
                if (DIGIT(c)) {
                        _tmpval = 10 * _tmpval + _sign * VALUE(c);
                        _state = wait_digits_or_comma_or_bracket;
                        if (_tmpval > 32767 || _tmpval < -32768)
                                set_error(parser_value_out_of_range);
                } else if (COMMA(c)) {
                        if (_length < 32) {
                                _value[_length++] = _tmpval;
                                _state = wait_minus_or_digit_vector;
                        } else {
                                set_error(parser_vector_too_long);
                        }
                } else if (CLOSEBRACKET(c)) {
                        if (_length < 32) {
                                _value[_length++] = _tmpval;
                                _state = wait_carriage_return;
                        } else {
                                set_error(parser_vector_too_long);
                        }
                } else {
                        set_error(parser_unexpected_char);
                }
                break;
        }

        return r;
}
