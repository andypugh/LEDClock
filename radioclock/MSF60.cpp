//#include <WProgram.h>
#include "RadioClock.h"
#include "MSF60.h"

/* ---- static defines private to this module -----*/

// list of FSM states
#define S_START_MESSAGE        2
#define S_START_BITS           3
#define S_END_BITS             4
#define S_1D                   5
#define S_2D                   6
#define S_3D                   7
#define S_5D                   8
#define S_1U                   9
#define S_1U1D                10

static const char * stateStr[] = { "S_INIT", "S_ERROR", "S_START_MESSAGE",
        "S_START_BITS", "S_END_BITS", "S_1D", "S_2D", "S_3D", "S_5D", "S_1U",
        "S_1U1D", "S_ERROR" };

        /* ----- end of static defines -----*/
        
const char * MSF60::state2Str(uint8_t state) {
    return stateStr[state];
}

// Convert buffer to Time.h struct
void MSF60::read(tmElements_t &tm) {
    tm.Second = 0;
    tm.Minute = bcd2dec(reverse(buffer.data.Min, 1));
    tm.Hour = bcd2dec(reverse(buffer.data.Hour, 2));
    tm.Wday = bcd2dec(reverse(buffer.data.Weekday, 5));
    tm.Day = bcd2dec(reverse(buffer.data.Day, 2));
    tm.Month = bcd2dec(reverse(buffer.data.Month, 3));
    tm.Year = y2kYearToTm(bcd2dec(reverse(buffer.data.Year, 0)));
}

// add a bit to the buffer, return false if an error occured
bool MSF60::addBits(uint8_t b1, uint8_t b2) {

    if (bitCount < 17) {
        // ignore prefix
    } else if (bitCount < 52) {
//        if (b2 != 0) // should we ignore the fix bits ??
//            return error(err_expected_0, bitCount);
        buffer.bits = buffer.bits | ((uint64_t) b1 << (bitCount));
        lastBit = b1;
    } else if (bitCount < 54) {
        // ignore  STW
    } else if (bitCount < 58) {
//        if (b1 != 1) // should we ignore the fix bits ??
//            return error(err_expected_1, bitCount);
        buffer.bits = buffer.bits | ((uint64_t) b2 << (bitCount));
        lastBit = b2;
    } else if (bitCount < 60) {
        // ignore ST
    } else {
        return error(err_too_many_bits, bitCount);
    }
    bitCount++;
    return true;
}

// check parity bits in buffer
bool MSF60::parityOk() {
    return (parity(buffer.data.Year) != buffer.data.P1)
            && ((parity(buffer.data.Day) + parity(buffer.data.Month)) % 2
                    != buffer.data.P2)
            && (parity(buffer.data.Weekday) != buffer.data.P3)
            && ((parity(buffer.data.Hour) + parity(buffer.data.Min)) % 2
                    != buffer.data.P4);
}

// FSM transitions

bool MSF60::doFSM() {

    switch (fsm_state) {
    case S_INIT:
        if (T_5D()) {
            fsm_state = S_START_MESSAGE;
            // init buffer
            bitCount = 0;
            buffer.bits = 0;
            return WAIT;
        } else
            return WAIT;
    case S_START_MESSAGE:
        if (T_5U()) {
            fsm_state = S_START_BITS;
            syncTime(1);
            addBits(0, 0);
             tStartMsg = tDown;
			unsetError(); // added my mem
            return WAIT;
        } else {
            return error(err_signal, uDuration);
        }
    case S_START_BITS:
        if (T_1D()) {
            fsm_state = S_1D;
            return WAIT;
        } else if (T_2D()) {
            fsm_state = S_2D;
            return WAIT;
        } else if (T_3D()) {
            fsm_state = S_3D;
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
    case S_1D:
        if (T_9U()) {
            fsm_state = S_END_BITS;
            return addBits(0, 0);
        } else if (T_1U()) {
            fsm_state = S_1U;
            return WAIT;
        } else {
            return error(err_signal, uDuration);
        }
    case S_1U:
        if (T_1D()) {
            fsm_state = S_1U1D;
            return WAIT;
        } else {
            return error(err_signal, dDuration);
        }
    case S_1U1D:
        if (T_7U()) {
            fsm_state = S_END_BITS;
            return addBits(0, 1);
        } else {
            return error(err_signal, uDuration);
        }
    case S_2D:
        if (T_8U()) {
            fsm_state = S_END_BITS;
            return addBits(1, 0);
        } else {
            return error(err_signal, uDuration);
        }
    case S_3D:
        if (T_7U()) {
            fsm_state = S_END_BITS;
            return addBits(1, 1);
        } else {
            return error(err_signal, uDuration);
        }
    case S_5D:
        if (T_5U()) {
            fsm_state = S_START_BITS;
            return WAIT;
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

MSF60::MSF60(int signalPin, bool invertedSignal, int signalLEDPin,
        int syncedLEDPin, int errorLEDPin, int onPin, int onLevel) :
        RadioClock(signalPin, invertedSignal, signalLEDPin, syncedLEDPin, errorLEDPin, onPin,
                onLevel) {

}
