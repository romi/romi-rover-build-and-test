#ifndef __PARSER_H
#define __PARSER_H

class Parser
{
protected:
        char _state;
        char _opcode;
        int16_t _value[8];
        char _length;
        char _text[32];
        unsigned char _textlen;
        int _tmpval;
        int _sign;
        const char* _opcodesWithValue;
        const char* _opcodesWithoutValue;

public:
        
        Parser(const char* opcodesWithValue,
               const char* opcodesWithoutValue) {
                _opcodesWithValue = opcodesWithValue;
                _opcodesWithoutValue = opcodesWithoutValue;
                _state = 0;
                _text[0] = 0;
                _text[127] = 0;
        }

        int value(int index = 0) {
                return _value[index];
        }

        char *text() {
                return _text;
        }

        int length() {
                return _length;
        }

        char opcode() {
                return _opcode;
        }
        
        bool process(char c);
};

#endif
