/* frameHandler.h
 *
 * Arduino library to read from Victron devices using VE.Direct protocol.
 * Derived from Victron framehandler reference implementation.
 * 
 * 2020.05.05 - 0.2 - initial release
 * 2021.02.23 - 0.3 - change frameLen to 22 per VE.Direct Protocol version 3.30
 * 
 */

#ifndef FRAMEHANDLER_H_
#define FRAMEHANDLER_H_


const uint8_t frameLen = 22;                       // VE.Direct Protocol: max frame size is 18
const uint8_t nameLen = 9;                         // VE.Direct Protocol: max name size is 9 including /0
const uint8_t valueLen = 33;                       // VE.Direct Protocol: max value size is 33 including /0
//const uint8_t buffLen = 40;                        // Maximum number of lines possible from the device. Current protocol shows this to be the BMV700 at 33 lines.

const uint8_t relevant_field_max = 9;

class VeDirectFrameHandler {

public:
    VeDirectFrameHandler();
    void rxData(uint8_t inbyte);                // byte of serial data to be passed by the application

    char veName[relevant_field_max][nameLen] = { };        // public buffer for received names
    int32_t veValue[relevant_field_max] = { };      // public buffer for received values

    int frameIndex;                             // which line of the frame are we on
    int veEnd;                                  // current size (end) of the public buffer

private:
    //bool mStop;                               // not sure what Victron uses this for, not using

    enum States {                               // state machine
        IDLE,
        RECORD_BEGIN,
        RECORD_NAME,
        RECORD_VALUE,
        CHECKSUM,
        RECORD_HEX
    };

    int mState;                                 // current state

    uint8_t	mChecksum;                          // checksum value

    char * mTextPointer;                        // pointer to the private buffer we're writing to, name or value

    char mName[9];                              // buffer for the field name
    char mValue[33];                            // buffer for the field value
    char tempName[frameLen][nameLen];           // private buffer for received names
    int32_t tempValue[frameLen];         // private buffer for received values

    void textRxEvent(char *, char *);
    void frameEndEvent(bool);
    void logE(char *, const char *);
    bool hexRxEvent(uint8_t);
    int32_t convertValue(char *, char *);
    bool is_relevant(char *name);
};

#endif // FRAMEHANDLER_H_
