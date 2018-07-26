/*
 * WWVB.h - library for WWVB time
 * This library is intended to be uses with Arduino Time.h library functions
 */

#include "WWVB.h"

// list of FSM states

#define S_START_MESSAGE   2
#define S_START_BITS      3
#define S_END_BITS        4
#define S_INIT_8D_1       5
#define S_INIT_2U         6
#define S_INIT_8D_2       7
#define S_2D              8
#define S_5D              9
#define S_8D             10

#define MARKER          128

static const char * stateStr[] = { "S_INIT", "S_ERROR", "S_START_MESSAGE",
		"S_START_BITS", "S_END_BITS", "S_INIT_8D_1", "S_INIT_2U", "S_INIT_8D_2",
		"S_2D", "S_5D", "S_8D" };

const char* WWVB::state2Str(byte state) {
	return stateStr[state];
}


// Convert buffer to Time.h struct
void WWVB::read(tmElements_t &tm) {
	time_t dayOfYear10 = bcd2dec(reverse(buffer.data.DayOfYearTen, 2));
	time_t dayOfYear = bcd2dec(reverse(buffer.data.DayOfYear, 4));
	uint8_t leapYear = buffer.data.LYI;
	dayOfYearToTm(dayOfYear + dayOfYear10 * 10, leapYear, tm);
	tm.Year = y2kYearToTm(bcd2dec(reverse(buffer.data.Year, 0)));

	tm.Minute = bcd2dec(reverse(buffer.data.Min, 1));
	tm.Hour = bcd2dec(reverse(buffer.data.Hour, 2));
	tm.Second = 0;
}

// add a bit to the buffer, return CONTINUE if an error occured
bool WWVB::addBit(uint8_t b) {

	if (bitCount >= 60)
		return error(err_too_many_bits, bitCount);
	if (bitCount == 0) {
		if (b != MARKER)
			return error(err_expected_marker, bitCount);
	}
	// check unused 0 bits
	else if (bitCount == 4 || bitCount == 10 || bitCount == 11 || bitCount == 14
			|| bitCount == 20 || bitCount == 21 || bitCount == 24
			|| bitCount == 34 || bitCount == 35 || bitCount == 44
			|| bitCount == 54) {
		if (b != 0)
			return error(err_expected_0, bitCount);
	}
	// check Marker bits
	else if (bitCount == 9 || bitCount == 19 || bitCount == 29 || bitCount == 39
			|| bitCount == 49) {
		if (b != MARKER)
			return error(err_expected_marker, bitCount);
	}
	else if (bitCount > 35 && bitCount < 44) {
		// ignore bit
	}
	else if (bitCount > 55 && bitCount < 60) {
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
// WWVB has no parity check
bool WWVB::parityOk() {
	return true;
}

// FSM transitions

bool WWVB::doFSM() {

	switch (fsm_state) {
	case S_INIT:
		if (T_8D()) {
			fsm_state = S_INIT_8D_1;
			return WAIT;
		} else
			return WAIT;
	case S_INIT_8D_1:
		if (T_2U()) {
			bitCount = 0;
			fsm_state = S_INIT_2U;
			return WAIT;
		} else {
			return error(err_signal, uDuration);
		}
	case S_INIT_2U:
		if (T_8D()) {
			fsm_state = S_INIT_8D_2;
			syncTime(0);
			return WAIT;
		} else {
			return error(err_signal, dDuration);
		}
	case S_INIT_8D_2:
		if (T_2U()) {
			fsm_state = S_START_MESSAGE;
			// init buffer
			bitCount = 0;
			bufferCount = 0;
			buffer.bits = (uint64_t) 0;
			tStartMsg = tUp;
			return addBit(MARKER);
		} else {
			return error(err_signal, uDuration);
		}
	case S_START_MESSAGE:
		fsm_state = S_START_BITS;
		return WAIT;
	case S_START_BITS:
		if (T_2D()) {
			fsm_state = S_2D;
			return WAIT;
		} else if (T_5D()) {
			fsm_state = S_5D;
			return WAIT;
		} else if (T_8D()) {
			fsm_state = S_8D;
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
	case S_2D:
		if (T_8U()) {
			fsm_state = S_END_BITS;
			return addBit(0);
		} else {
			return error(err_signal, uDuration);
		}
	case S_5D:
		if (T_5U()) {
			fsm_state = S_END_BITS;
			return addBit(1);
		} else {
			return error(err_signal, uDuration);
		}
	case S_8D:
		if (T_2U()) {
			fsm_state = S_END_BITS;
			return addBit(MARKER);
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

WWVB::WWVB(int signalPin, bool invertedSignal, int signalLEDPin,
		int syncedLEDPin, int errorLEDPin, int onPin, int onLevel) :
		RadioClock(signalPin, invertedSignal, signalLEDPin, syncedLEDPin,
				errorLEDPin, onPin, onLevel) {
}

