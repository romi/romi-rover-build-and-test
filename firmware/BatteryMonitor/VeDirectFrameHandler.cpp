/* framehandler.cpp
 *
 * Arduino library to read from Victron devices using VE.Direct protocol.
 * Derived from Victron framehandler reference implementation.
 * 
 * The MIT License
 * 
 * Copyright (c) 2019 Victron Energy BV
 * Portions Copyright (C) 2020 Chris Terwilliger
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *  
 * 2020.05.05 - 0.2 - initial release
 * 2020.06.21 - 0.2 - add MIT license, no code changes
 * 2020.08.20 - 0.3 - corrected #include reference
 * 
 */

#include <Arduino.h>
#include "VeDirectFrameHandler.h"


//#define MODULE "VE.Frame"	// Victron seems to use this to find out where logging messages were generated
//
// The name of the record that contains the checksum.
static constexpr char MODULE[] = "VE.Frame";
static constexpr char checksumTagName[] = "CHECKSUM";
static constexpr char NAME_ALARM[] = "ALARM";
static constexpr char NAME_RELAY[] = "RELAY";
static constexpr char OFF[] = "OFF";
static constexpr char CHECKSUM_INVALID[] = "[CHECKSUM] Invalid frame";

static constexpr char relavantFields[relevant_field_max][nameLen] = { "V", "VS", "I", "P", "CE", "SOC", "TTG", "ALARM", "AR" };

VeDirectFrameHandler::VeDirectFrameHandler() :
	//mStop(false),	// don't know what Victron uses this for, not using
        veName(),
        veValue(),
        frameIndex(0),
        veEnd(0),
        mState(IDLE),
        mChecksum(0),
        mTextPointer(nullptr),
        mName(),
        mValue(),
        tempName(),
        tempValue()
{
}

/*
 *	rxData
 *  This function is called by the application which passes a byte of serial data
 *  It is unchanged from Victron's example code
 */
void VeDirectFrameHandler::rxData(uint8_t inbyte)
{
	//if (mStop) return;
	if ( (inbyte == ':') && (mState != CHECKSUM) ) {
		mState = RECORD_HEX;
	}
	if (mState != RECORD_HEX) {
        int tmp(mChecksum);
        tmp+= inbyte;
        mChecksum = static_cast<uint8_t>(tmp);
	}
	inbyte = static_cast<uint8_t>(toupper(inbyte));

	switch(mState) {
	case IDLE:
		/* wait for \n of the start of an record */
		switch(inbyte) {
		case '\n':
			mState = RECORD_BEGIN;
			break;
		case '\r': /* Skip */
		default:
			break;
		}
		break;
	case RECORD_BEGIN:
		mTextPointer = mName;
		*mTextPointer++ = (char)inbyte;
		mState = RECORD_NAME;
		break;
	case RECORD_NAME:
		// The record name is being received, terminated by a \t
        if (inbyte == '\t')
        {
            // the Checksum record indicates a EOR
            if ( mTextPointer < (mName + sizeof(mName)) ) {
                *mTextPointer = 0; /* Zero terminate */
                if (strcmp(mName, checksumTagName) == 0) {
                    mState = CHECKSUM;
                    break;
                }
            }
            mTextPointer = mValue; /* Reset value pointer */
            mState = RECORD_VALUE;
        }
        else
        {
            // add byte to name, but do no overflow
            if ( mTextPointer < (mName + sizeof(mName)) )
                *mTextPointer++ = (char)inbyte;
        }
		break;
	case RECORD_VALUE:
		// The record value is being received.  The \r indicates a new record.
		switch(inbyte) {
		case '\n':
			// forward record, only if it could be stored completely
			if ( mTextPointer < (mValue + sizeof(mValue)) ) {
				*mTextPointer = 0; // make zero ended
				textRxEvent(mName, mValue);
			}
			mState = RECORD_BEGIN;
			break;
		case '\r': /* Skip */
			break;
		default:
			// add byte to value, but do no overflow
			if ( mTextPointer < (mValue + sizeof(mValue)) )
				*mTextPointer++ = (char)inbyte;
			break;
		}
		break;
	case CHECKSUM:
	{
		bool valid = mChecksum == 0;
    
//		if (!valid)
//			logE(MODULE,CHECKSUM_INVALID);
		mChecksum = 0;
		mState = IDLE;
		frameEndEvent(valid);
		break;
	}
	case RECORD_HEX:
		if (hexRxEvent(inbyte)) {
			mChecksum = 0;
			mState = IDLE;
		}
		break;
        default:
            break;
	}

}

/*
 * textRxEvent
 * This function is called every time a new name/value is successfully parsed.  It writes the values to the temporary buffer.
 */
void VeDirectFrameHandler::textRxEvent(char * name, char * value) {
    if (is_relevant(name))
    {
        strcpy(tempName[frameIndex], name);    // copy name to temporary buffer
        tempValue[frameIndex] = convertValue(name, value);
        frameIndex++;
    }
}

/*
 * textRxEvent
 * This function is called every time a new name/value is successfully parsed.  It writes the values to the temporary buffer.
 */
int32_t VeDirectFrameHandler::convertValue(char * name, char * value) {

    int32_t converted = 0;
    if ((strcmp(NAME_ALARM, name) == 0) ||
        (strcmp(NAME_RELAY, name) == 0))
    {
        converted = (strcmp(OFF, value) == 0) ? 0 : 1;
    }
    else
    {
        converted = atoi(value);
    }
    return converted;
}

/*
 *	frameEndEvent
 *  This function is called at the end of the received frame.  If the checksum is valid, the temp buffer is read line by line.
 *  If the name exists in the public buffer, the new value is copied to the public buffer.	If not, a new name/value entry
 *  is created in the public buffer.
 */
void VeDirectFrameHandler::frameEndEvent(bool valid) {
    if ( valid ) {
        for ( int i = 0; i < frameIndex; i++ ) {				// read each name already in the temp buffer
            bool nameExists = false;
            for ( int j = 0; j <= veEnd; j++ ) {				// compare to existing names in the public buffer
                if ( strcmp(tempName[i], veName[j]) == 0 ) {
                    veValue[j] = tempValue[i];			// overwrite tempValue in the public buffer
                    nameExists = true;
                    break;
                }
            }
            if ( !nameExists ) {
                strcpy(veName[veEnd], tempName[i]);				// write new Name to public buffer
                veValue[veEnd] = tempValue[i];			// write new Value to public buffer
                veEnd++;										// increment end of public buffer
                if ( veEnd >= relevant_field_max ) {						// stop any buffer overrun
                    veEnd = relevant_field_max - 1;
                }
            }
        }
    }
    frameIndex = 0;	// reset frame
}

/*
 *	logE
 *  This function included for continuity and possible future use.	
 */
void VeDirectFrameHandler::logE(char * module, const char * error) {
    (void) module;
    (void) error;
	//Serial.print("MODULE: ");
    //Serial.println(module);
    //Serial.print("ERROR: ");
    //Serial.println(error);
	return;
}

/*
 *	hexRxEvent
 *  This function included for continuity and possible future use.	
 */
bool VeDirectFrameHandler::hexRxEvent(uint8_t inbyte) {
    (void)inbyte;
	return true;		// stubbed out for future
}

bool VeDirectFrameHandler::is_relevant(char *name) {
    bool relevant = false;
    for (auto relavantField : relavantFields){
        if(strcmp(relavantField, name) == 0) {
            relevant = true;
            break;
        }
    }
    return relevant;
}
