/*
 * JJY.h - library for JJY time
 * This library is intended to be uses with Arduino Time.h library functions
 */

#include "JJY.h"

// list of FSM states

#define S_START_MESSAGE   2
#define S_START_BITS      3
#define S_END_BITS        4
#define S_INIT_2U         5
#define S_INIT_M_1        6
#define S_INIT_2U_2       7
#define S_2U              8
#define S_5U              9
#define S_8U             10


#define MARKER          128

static const char * stateStr[] = { "S_INIT", "S_ERROR", "S_START_MESSAGE",
    "S_START_BITS", "S_END_BITS", "S_INIT_2U", "S_INIT_M_1", "S_INIT_2U_2", "S_2U", "S_5U", "S_8U"};
	

const char* JJY::state2Str(byte state) {
    return stateStr[state];
}


// Convert buffer to Time.h struct
void JJY::read(tmElements_t &tm) {
    time_t dayOfYear10 = bcd2dec(reverse(buffer.data.DayOfYearTen, 2));
    time_t dayOfYear = bcd2dec(reverse(buffer.data.DayOfYear, 4));
    int yr = 2000+ bcd2dec(reverse(buffer.data.Year, 0));
    dayOfYearToTm(dayOfYear + dayOfYear10*10, isLeapYear(yr), tm);

    tm.Year = y2kYearToTm(bcd2dec(reverse(buffer.data.Year, 0)));
    tm.Minute = bcd2dec(reverse(buffer.data.Min, 1));
    tm.Hour = bcd2dec(reverse(buffer.data.Hour, 2));
    tm.Second = 0;
}

// add a bit to the buffer, return CONTINUE if an error occured
bool JJY::addBit(uint8_t b) {
    if (bitCount >= 60)
        return error(err_too_many_bits, bitCount);
	if (bitCount == 0) {
		if (b != MARKER)
			return error(err_expected_marker, bitCount);
	}

    // check unused 0 bits
	else if (bitCount == 4 || bitCount == 10 || bitCount == 11 || bitCount == 14
            || bitCount == 20|| bitCount == 21 || bitCount == 24
            || bitCount == 34 || bitCount == 35 || bitCount == 38 || bitCount == 40 || bitCount == 55
            || bitCount == 56 || bitCount == 57 || bitCount == 58 ) {
        if (b != 0)
            return error(err_expected_0, bitCount);
    }
    // check Marker bits
    else if (bitCount == 9 || bitCount == 19 || bitCount == 29
            || bitCount == 39
            || bitCount == 49) {
        if (b != MARKER)
            error(err_expected_marker, bitCount);
    } else if (bitCount == 38 || bitCount > 52 ) {
        // ignore bit
    }
    else // add bit
    {
        buffer.bits = buffer.bits | ((uint64_t) b << bufferCount);
        lastBit = b;
        bufferCount++;
    }

    lastBit = b;

    bitCount++;

    return CONTINUE;
}

// check parity bits in buffer
bool JJY::parityOk() {
    return (parity(buffer.data.Hour) == buffer.data.PA1)
    		&& (parity(buffer.data.Min) == buffer.data.PA2);
}

bool JJY::isLeapYear(int year)
{
	if (year % 4 == 0)
	       if (year % 100 == 0)
	               if (year % 400 == 0)
	                   return true;
	               else
	                   return false;
	       else
	    	   return true;
	else
		return false;
}

// FSM transitions

bool JJY::doFSM() {

    switch (fsm_state) {
    case S_INIT:
        if (T_2U()) {
            fsm_state = S_INIT_2U;
            return WAIT;
        } else
            return WAIT;
    case S_INIT_2U:
        if (T_8D()) {
			bitCount = 0;
            fsm_state = S_INIT_M_1;
            return WAIT;
        } else {
            return error(err_signal, uDuration);
        }
    case S_INIT_M_1:
        if (T_2U()) {
            fsm_state = S_INIT_2U_2;
			syncTime(0);
            return WAIT;
        } else {
            return error(err_signal, dDuration);
        }
    case S_INIT_2U_2:
        if (T_8D()) {
            fsm_state = S_START_MESSAGE;
            // init buffer
            bitCount = 0;
            bufferCount = 0;
            buffer.bits = (uint64_t)0;
            tStartMsg = tUp;
            return addBit(MARKER);
        } else {
            return error(err_signal, uDuration);
        }
    case S_START_MESSAGE:
            fsm_state = S_START_BITS;
            return WAIT;

    case S_START_BITS:
        if (T_2U()) {
            fsm_state = S_2U;
            return WAIT;
        } else if (T_5U()) {
            fsm_state = S_5U;
            return WAIT;
        } else if (T_8U()) {
            fsm_state = S_8U;
            return WAIT;
        } else {
            return error(err_signal, dDuration);

        }
    case S_END_BITS:
        if (T_MESSAGE_COMPLETE()) {
            if (parityOk()) {
                syncTimeInternal();
                fsm_state = S_INIT;
                return WAIT;
            } else {
                return error(err_parity, -1);
            }

        } else if (T_EXIT_60_SECS()) {
            return error(err_msg_duration, -1);
        } else {
            fsm_state = S_START_BITS;
            return WAIT;
        }
    case S_2U:
        if (T_8D()) {
            fsm_state = S_END_BITS;
            return addBit(MARKER);
        } else {
            return error(err_signal, uDuration);
        }
    case S_5U:
        if (T_5D()) {
            fsm_state = S_END_BITS;
            return addBit(1);
        } else {
            return error(err_signal, uDuration);
        }
    case S_8U:
        if (T_2D()) {
            fsm_state = S_END_BITS;
            return addBit(0);
        } else {
            return error(err_signal, uDuration);
        }
    case S_ERROR:
        fsm_state = S_INIT;
        return WAIT;
    default:
        return error(err_unknown_state, fsm_state);
    }

}

JJY::JJY(int signalPin, bool invertedSignal, int signalLEDPin,
        int syncedLEDPin, int errorLEDPin, int onPin, int onLevel) :
        RadioClock(signalPin, invertedSignal, signalLEDPin, syncedLEDPin, errorLEDPin, onPin, onLevel) {
}


