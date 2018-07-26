/*
 * RadioClock.cpp - library for RadioClock time
 * This library is intended to be used with Arduino Time.h library functions
 */

#include "RadioClock.h"

// time thresholds
#define MIN_1     25
#define MAX_1     170
#define MIN_2     140
#define MAX_2     260
#define MIN_3     240
#define MAX_3     370
#define MIN_5     370
#define MAX_5     580
#define MIN_7     580
#define MAX_7     800
#define MIN_8     700
#define MAX_8     950
#define MIN_9     800
#define MAX_9     1000
#define MIN_18    1700
#define MAX_18    1850
#define MIN_19    1850
#define MAX_19    2000
#define MIN_18_19 1700
#define MAX_18_19 2000
#define MIN_60    63000

#define SIGNAL_ERROR_MILLIS  3000
#define MIN_SIGNAL_CHANGE 50

// uncomment to show status in diags()
#define SHOW_SIGNAL false
#define SHOW_FSM_STATE false
#define SHOW_SIGNAL_TIMING false
#define SHOW_SIGNAL_TIMING_1 false
#define SHOW_BITS false

// state fields
volatile byte fsm_state        = 0;
volatile long lastSignalChange = 0;

setTime_t timeCallback;

RadioClock* _radioClock;

void _signalChange() {
    if (_radioClock != NULL)
        _radioClock->signalChange();
}

// error in signal
// always return FSM CONTINUE
bool RadioClock::error(int msg, int value) {
    if(status != status_acquiring) // status remains status_acquiring until the first sync
	    status = status_error;
    if (msg != err_missing_signal_change)
        fsm_state = S_ERROR;
    syncReady = false;
    errMsg = msg;
    errValue = value;
    if (errorState)
        return CONTINUE;
    errorState = true;
	setLEDs(); // show error and sync status on leds if enabled
    return CONTINUE;
}

void RadioClock::unsetError() {
    if (!errorState)
        return;

    errorState = false;
	setLEDs(); // show error and sync status on leds if enabled

}

// sets leds to indicate sync and error states
void RadioClock::setLEDs(){
    if (_errorLEDPin != NO_PIN)
       digitalWrite(_errorLEDPin, errorState);
    if (_syncedLEDPin != NO_PIN)
        digitalWrite(_syncedLEDPin, status == status_synced);	   
}


// Untility methods

// reverse the bits in the byte then shift by the number of bits
// we may need to shift because our numbers may have less than 8 bits
uint8_t RadioClock::reverse(uint8_t b, uint8_t shift) {
    return ((b * 0x0202020202ULL & 0x010884422010ULL) % 1023) >> shift;
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t RadioClock::bcd2dec(uint8_t bcd) {
    return bcd - ((bcd / 16) * 6);
}

// calculate the parity of a byte
uint8_t RadioClock::parity(uint8_t b) {
    return (((b * 0x0101010101010101ULL) & 0x8040201008040201ULL) % 0x1FF) & 1;
}


void RadioClock::dayOfYearToTm(time_t doy, uint8_t leapYear, tmElements_t &tm)
{
    int month = 0;
    int day = 0;

    if (leapYear == 0)
    {
        if (doy >= 335)
        {
            month = 12;
            day = doy - 334;
        }
        else if (doy >= 305)
        {
            month = 11;
            day = doy - 304;
        }
        else if (doy >= 274)
        {
            month = 10;
            day = doy - 273;
        }
        else if (doy >= 244)
        {
            month = 9;
            day = doy - 243;
        }
        else if (doy >= 213)
        {
            month = 8;
            day = doy - 212;
        }
        else if (doy >= 182)
        {
            month = 7;
            day = doy - 181;
        }
        else if (doy >= 152)
        {
            month = 6;
            day = doy - 151;
        }
        else if (doy >= 121)
        {
            month = 5;
            day = doy - 120;
        }
        else if (doy >= 91)
        {
            month = 4;
            day = doy - 90;
        }
        else if (doy >= 60)
        {
            month = 3;
            day = doy - 59;
        }
        else if (doy >= 32)
        {
            month = 2;
            day = doy - 31;
        }
        else if (doy >= 1)
        {
            month = 1;
            day = doy;
        }
    }
    else
    {
        if (doy >= 336)
        {
            month = 12;
            day = doy - 335;
        }
        else if (doy >= 306)
        {
            month = 11;
            day = doy - 305;
        }
        else if (doy >= 273)
        {
            month = 10;
            day = doy - 274;
        }
        else if (doy >= 245)
        {
            month = 9;
            day = doy - 244;
        }
        else if (doy >= 214)
        {
            month = 8;
            day = doy - 213;
        }
        else if (doy >= 183)
        {
            month = 7;
            day = doy - 182;
        }
        else if (doy >= 153)
        {
            month = 6;
            day = doy - 152;
        }
        else if (doy >= 122)
        {
            month = 5;
            day = doy - 121;
        }
        else if (doy >= 92)
        {
            month = 4;
            day = doy - 91;
        }
        else if (doy >= 61)
        {
            month = 3;
            day = doy - 60;
        }
        else if (doy >= 32)
        {
            month = 2;
            day = doy - 31;
        }
        else if (doy >= 1)
        {
            month = 1;
            day = doy;
        }

    }
    tm.Day = day;
    tm.Month = month;
}



// Finite State Machine (FSM)
// State change triggers
bool RadioClock::T_1D() {
    return (dDuration > MIN_1) && (dDuration <= MAX_1);
}
bool RadioClock::T_2D() {
    return (dDuration > MIN_2) && (dDuration <= MAX_2);
}
bool RadioClock::T_3D() {
    return (dDuration > MIN_3) && (dDuration <= MAX_3);
}
bool RadioClock::T_5D() {
    return (dDuration > MIN_5) && (dDuration <= MAX_5);
}
bool RadioClock::T_8D() {
    return (dDuration > MIN_8) && (dDuration <= MAX_8);
}
bool RadioClock::T_1U() {
    return (uDuration > MIN_1) && (uDuration <= MAX_1);
}
bool RadioClock::T_2U() {
    return (uDuration > MIN_2) && (uDuration <= MAX_2);
}
bool RadioClock::T_5U() {
    return (uDuration > MIN_5) && (uDuration <= MAX_5);
}
bool RadioClock::T_7U() {
    return (uDuration > MIN_7) && (uDuration <= MAX_7);
}
bool RadioClock::T_8U() {
    return (uDuration > MIN_8) && (uDuration <= MAX_8);
}
bool RadioClock::T_9U() {
    return (uDuration > MIN_9) && (uDuration <= MAX_9);
}
bool RadioClock::T_18U() {
    return (uDuration > MIN_18) && (uDuration <= MAX_18);
}
bool RadioClock::T_19U() {
    return (uDuration > MIN_19) && (uDuration <= MAX_19);
}
bool RadioClock::T_18_19U() {
    return (uDuration > MIN_18_19) && (uDuration <= MAX_18_19);
}
bool RadioClock::T_MESSAGE_COMPLETE() {
    return bitCount == 59;
}
bool RadioClock::T_EXIT_60_SECS() {
    long d = tUp - tStartMsg;
    return (d > MIN_60);
}

const char * RadioClock::state2Str(byte state) {
    return "?";
}

void RadioClock::printState() {
    static byte lastState = -99;
    if (lastState != fsm_state) {
        Serial.print("State Changed: ");
        Serial.print(state2Str(lastState));
        Serial.print(" -> ");
        Serial.println(state2Str(fsm_state));
        lastState = fsm_state;
    }
}

uint64_t RadioClock::getBuffer() {
    return (uint64_t) 0;
}

// todo: this would be more robust if it disabled interrupts and made a local copy of all variables that can be changed in the ISR
// but as this does not impact the accuracy of the clock, its  probably not be necessary 
void RadioClock::diags() // poll this to show decode status and report errors
{
    if (errMsg != noError) // if there is an undisplayed error, show it
    {
        Serial.print(errorMsg[errMsg]);
        if (errValue != -1) {
            Serial.print(" ");
            Serial.println(errValue, DEC);
        } else
            Serial.println();

        errMsg = noError;
        errValue = -1;
    }
    if (SHOW_SIGNAL) {

        static long lastUD = 0;
        static long lastDD = 0;
        if (dDuration > 0 && dDuration != lastDD) {
            Serial.print("Signal: L ");
            Serial.println(dDuration, DEC);
            lastDD = dDuration;
        }
        if (uDuration > 0 && uDuration != lastUD) {
            Serial.print("Signal: H ");
            Serial.println(uDuration, DEC);
            lastUD = uDuration;
        } else if (dDuration == 0 && uDuration == 0)
            Serial.println("Signal: ?");
    }

    if (SHOW_BITS) {
        static uint8_t prevBitcount = -1;
        if (prevBitcount != bitCount && bitCount > 0) {
            Serial.print("Bit Buffer [");
            Serial.print(bitCount-1, DEC);
            Serial.print("] =");
            Serial.println(lastBit, DEC);

        }
        prevBitcount = bitCount;

    }
    if (SHOW_FSM_STATE)
        printState();

}

// interrupt function called on input change
void RadioClock::signalChange() {

    // read in the current signal
    uint8_t in = digitalRead(_signalPin);
    // remember the current time
    currentTime = millis();
    unsigned long d = (currentTime - lastSignalChange);

    if (SHOW_SIGNAL_TIMING_1) {
        Serial.print(in, DEC);
        Serial.print(" ");
        Serial.println(d, DEC);
    }

    // ignore if too short, may be caused by noise
    if (d < MIN_SIGNAL_CHANGE)
        return;
    //lastSignalChange = currentTime;

    // if signal has not changed -> ignore
    // should not happen, but does
    if (lastIn == in)
        return;
    lastIn = in;

    // if required invert
    if (_invertedSignal)
        in = !in;

    // high or low signal ?
    if (in == 0) {
        tDown = currentTime;
        uDuration = (currentTime - tUp);
        dDuration = 0;
        if (SHOW_SIGNAL_TIMING) {
            Serial.print("H ");
            Serial.println(uDuration, DEC);
        }

    } else {
        tUp = currentTime;
        dDuration = (currentTime - tDown);
        uDuration = 0;
        if (SHOW_SIGNAL_TIMING) {
            Serial.print("L ");
            Serial.println(dDuration, DEC);
        }
    }

    // run the FSM
    while (doFSM())
        ;

    // show that we received a signal
    state = !state;
    if (_signalLEDPin != NO_PIN)
        digitalWrite(_signalLEDPin, state);
    setLEDs(); // update sync and error leds		

}

// implemented in sub classes
bool RadioClock::doFSM() {
    return false;
}

bool RadioClock::start() {
	Serial.println("start 1");
    if (_radioClock != NULL)
        return true; // already started
	Serial.println("start 2");

    // initialize fields    
    state = LOW;
    tUp = 0;
    tDown = 0;
    tOldDown = 0;
    tStartMsg = 0;
    bitCount = 0;
    lastIn = -1;
    errorState = false;
    errMsg = noError;
    errValue = -1;


    _radioClock = this;
    if (_onPin != -1)
        digitalWrite(_onPin, _onLevel);
	setLEDs(); // show error and sync status on leds if enabled
    if (_signalPin == 2)
        attachInterrupt(0, _signalChange, CHANGE);
    else if (_signalPin == 3)
        attachInterrupt(1, _signalChange, CHANGE);
    else
        return false;	
	Serial.println("start 3");
    status = status_acquiring;
    return true;

}

void RadioClock::stop() {
	Serial.println("stop 1");
    if (_radioClock == NULL)
        return;
	Serial.println("stop 2");
    errorState = false;		
    if (_onPin != NO_PIN)
        digitalWrite(_onPin, !_onLevel);
    setLEDs(); // show error and sync status on leds if enabled      
    if (_signalPin == 2)
        detachInterrupt(0);
    else if (_signalPin == 3)
        detachInterrupt(1);
    status = status_stopped;
    _radioClock = NULL;
	Serial.println("stop 3");

}

bool RadioClock::isStarted() {
    return  status != status_stopped;
}

uint8_t RadioClock::getTickCount() {
    return bitCount;
}

void RadioClock::setTimeZoneOffset(int hours) {
    _tzOffset = hours;
}

radioStatus_t RadioClock::getStatus() {
	if (status == status_stopped)
		return status;
    if (errorState)
        return status_error;
    // sanity check so we can set error LED if no interrupts received
    long d = (millis() - tDown);
    if (d > SIGNAL_ERROR_MILLIS) {
    	  // no signal change for too long ->
        // turn on the error LED and deactivate the cache
        error(err_missing_signal_change, (int) d);
    }
    return status;
}

time_t RadioClock::getTime() {
    // returns the current time if synced, else returns 0
    // uses bitcount to provide seconds 
    time_t result = 0L;
    if( status == status_synced)
    {
       // disable interrupts and get a copy of the time and count
       uint8_t oldSREG = SREG;
       cli();
       time_t t = syncedTime ;  
       uint8_t c = bitCount;  
       SREG = oldSREG;  
       result =  t + c + (_tzOffset * 3600L);
       status = status_synced;
    }
    return result;
}

void RadioClock::setTimeCallback( setTime_t callback){
   timeCallback = callback; 
}

RadioClock::RadioClock(int signalPin, bool invertedSignal, int signalLEDPin,
       int syncedLEDPin, int errorLEDPin, int onPin, int onLevel) {
    _signalLEDPin = signalLEDPin;
    _errorLEDPin = errorLEDPin;
    _syncedLEDPin = syncedLEDPin;
    _invertedSignal = invertedSignal;
    _onPin = onPin;
    _onLevel = onLevel;
    _signalPin = signalPin;
    if (signalLEDPin != NO_PIN)
        pinMode(signalLEDPin, OUTPUT);		
    if (errorLEDPin != NO_PIN)
        pinMode(errorLEDPin, OUTPUT);
    if (_syncedLEDPin != NO_PIN)
	    pinMode(errorLEDPin, OUTPUT);
        digitalWrite(_syncedLEDPin, LOW);        
    if (onPin != NO_PIN) {
        pinMode(onPin, OUTPUT);
        digitalWrite(onPin, !onLevel);
    }
    if ( !(_signalPin == 2 || _signalPin == 3)) {
        error(err_wrong_signal_pin, _signalPin);
    }
	status = status_stopped;
}

// implemented in sub classes
void RadioClock::read(tmElements_t &tm) {
}

// store the time
void RadioClock::syncTimeInternal() {
    unsetError();
    tmElements_t tm;
    read(tm);    
    syncedTime = makeTime(tm);
    syncReady = true;

    }

// sync with system time if a callback has been set
// add the time difference with the begin of minute signal
void RadioClock::syncTime(int secs) {
    if(syncReady && timeCallback != NULL) {
       timeCallback(syncedTime + (_tzOffset * 3600L) + secs);
    syncReady = false;
    status = status_synced;
}


}

