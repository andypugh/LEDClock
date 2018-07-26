#include "RadioClock.h"
#include "DCF77.h"


/* ---- static defines private to this module -----*/

// list of FSM states
#define S_START_MESSAGE        2
#define S_START_BITS           3
#define S_END_BITS             4
#define S_1D                   5
#define S_2D                   6
#define S_BIT0                 7

static const char * stateStr[] = { "S_INIT", "S_ERROR", "S_START_MESSAGE",
        "S_START_BITS", "S_END_BITS", "S_1D", "S_2D", "S_BIT0"};
        

/* ----- end of static defines -----*/

const char * DCF77::state2Str(byte state) {
    return stateStr[state];
}

// Convert buffer to Time.h struct
void DCF77::read(tmElements_t &tm) {
      tm.Second = 0;
      tm.Minute = bcd2dec(buffer.data.Min);
      tm.Hour =   bcd2dec(buffer.data.Hour);
      tm.Wday = bcd2dec(buffer.data.Weekday);
      tm.Day = bcd2dec(buffer.data.Day);
      tm.Month = bcd2dec(buffer.data.Month);
      tm.Year = y2kYearToTm((bcd2dec(buffer.data.Year)));
}

// add a bit to the buffer, return false if an error occured
bool DCF77::addBit(uint8_t b) {

    if (bitCount<60)
    {
        buffer.bits = buffer.bits | ((uint64_t) b << bitCount);
        lastBit = b;
    } else
    {
        return error(err_too_many_bits, bitCount);
    }
    bitCount++;
    return CONTINUE;
}

// check parity bits in buffer
bool DCF77::parityOk()
{
    return (parity(buffer.data.Min) == buffer.data.P1) && (parity(buffer.data.Hour) == buffer.data.P2)
           && ( (parity(buffer.data.Day) + parity(buffer.data.Weekday) + parity(buffer.data.Month) + parity(buffer.data.Year)) %2 == buffer.data.P3);
}

// FSM transitions

bool DCF77::doFSM() {

    switch (fsm_state) {
    case S_INIT:
        if (T_18_19U()) {
            fsm_state = S_START_MESSAGE;
            // init buffer
            bitCount = 0;
            buffer.bits = 0;
            syncTime(0);
            return WAIT;
        } else
            return WAIT;
    case S_START_MESSAGE:
        if (T_1D()) {
            fsm_state = S_BIT0;
            tStartMsg = tDown;
            unsetError(); // added my mem
            return WAIT;
        } else {
            return error(err_signal, dDuration);
        }
    case S_BIT0:
        if (T_9U()) {
            fsm_state = S_START_BITS;
            addBit(0);
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
        } else {
            return error(err_signal, dDuration);

        }
    case S_END_BITS:
        if (T_MESSAGE_COMPLETE()) {
            if (parityOk()) {
                syncTimeInternal();
                fsm_state = S_INIT;
                return CONTINUE;
            } else {
            	syncReady = false;
                return error(err_parity, -1);
            }

        } else if (T_EXIT_60_SECS()) {
            return error(err_msg_duration, -1);
        } else {
            fsm_state = S_START_BITS;
            return WAIT;
        }
    case S_1D:
        if (T_9U() || T_19U()) {
            fsm_state = S_END_BITS;
            return addBit(0);
        } else {
            return error(err_signal, uDuration);
        }
    case S_2D:
        if (T_8U() || T_18U()) {
            fsm_state = S_END_BITS;
            return addBit(1);
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

DCF77::DCF77(int signalPin, bool invertedSignal, int signalLEDPin,
        int syncedLEDPin, int errorLEDPin, int onPin, int onLevel) :
        RadioClock(signalPin, invertedSignal, signalLEDPin, syncedLEDPin, errorLEDPin, onPin, onLevel) {
}

