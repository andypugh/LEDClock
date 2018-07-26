/* mem 
Made the following changes to be consistant with Arduino library convention:
- changed arduino types to C++ types (boolean ->bool, byte -> uint8_t)
- replaced tabs with spaces 
- added defines so code cn compile in Arduino 1.0 IDE

// these changes are optional but I think they are clearer for users
- added define for NO_PIN to be used instead of -1 in constructor
- changed inversed to the more common english term: inverted
- changed getBitCount to getTickCount (see method comment for more info)
- changed isOk to getStatus
- added setTime callback and removed cache
- added method to return sync status (polling this will set the error LED state)



Options if you like (I will only finalize these changes if you are happy with them)
- change state and error defines to enums
- if the goal is to have the interface for all of the derived classes the same then I suggest we remove the interface comments from the headers
and move into a readme file so we don't have to update docs in multiple place if the interface changes.
*/

/*
 RadioClock.h - Base Class for decoding RadioClock time signals to time_t for use with Arduino Time.h library

 Inspired by the Arduino DCF77 decoder v0.2 by Mathias Dalheimer (md@gonium.net)
 Updated decoding logic based on work by Ron (http://sourceforge.net/users/rzorzorzo)
 Copyright (c) (your name and my name goes here) 2011
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA  

 */

#ifndef RadioClock_h
#define RadioClock_h

#include <Arduino.h>

#include <Time.h>

#define NO_PIN -1 // value indicating that no pin should be assigned

// common states
#define S_INIT                       0
#define S_ERROR                      1

// doFSM return values
#define WAIT             false
#define CONTINUE         true

// status return codes
typedef enum { status_stopped,    // the system is stopped  
               status_acquiring,  // the time has not yet been synced 
               status_synced ,    // the time is synced within the last minute
               status_error       // an error occured                                             
             } radioStatus_t;

// error states
#define   noError                  -1
#define   err_expected_0            0
#define   err_expected_1            1
#define   err_expected_marker       2
#define   err_too_many_bits         3
#define   err_state_transition      4
#define   err_wrong_signal_pin      5
#define   err_missing_signal_change 6
#define   err_unknown_state         7
#define   err_signal                8
#define   err_parity                9
#define   err_msg_duration         10

static const char * errorMsg[] = {
        "Expected bit 0, got 1",
        "Expected bit 1, got 0",
        "Expected marker",
        "Received too many bits, got:",
        "Error in state transition",
        "Error signal pin should be 2 or 3: ",
        "Error: pulse too short: ",
        "Error: unknown state: ",
        "Error: unexpected signal duration: ",
        "Error: parity error",
        "Error: message duration too long" };

typedef void (*setTime_t)(const time_t);  // callback function to set the time

// library interface description
class RadioClock {
    // user-accessible "public" interface
public:
    // signalLedPin: output pin to a signal LED. When signal is received LED blinks.
    //               set to -1 if not required. 
    // errorLEDPin: output pin to a signal LED. LED is turned on if message start has not been detected
    //              or an error parsing the message has occured .
    //              Set to -1 if not required
    // signalPin: digital input pin attached to module output. must be 2 or 3.
    // invertedSignal: some modules deliver an inverted signal, some require a transistor to amplify the signal, which may invert it.
    // onPin: some modules have an input for (PON) for turning the receiver on/off. this is the corresponding output pin. 
    //       set to -1 if not required
    // onLevel: LOW/HIGH indicates how to set the onPin to turn the receiver on.
    RadioClock(int signalPin, bool invertedSignal = false, int signalLEDPin = NO_PIN,
       int syncedLEDPin = NO_PIN, int errorLEDPin = NO_PIN, int onPin = NO_PIN, int onLevel = -1);

    // get the time from the Radio Clock Module. Returns 0 if no time has been received or an error has occured
    time_t getTime();
    
    // the give method will be called to set the system time when the radio clock is synced
    void setTimeCallback( setTime_t callback);

    // start the reception. 
    // if onPin has been set the receiver is turned on.
    // attaches the interrupt to signalPin
    // returns false if unable to start (caused by invalid pin arguments)
    bool start();

    // stop the reception.
    // if onPin has been set the receiver is turned off
    // detaches the interrupt to signalPin
    void stop();

    // set offset in hours to the current time zone (for example: uk = -1, when using DCF77)
    void setTimeZoneOffset(int hours);

    // return true if start() was called
    bool isStarted();

    // return indicates if time is valid, error or still acquiring
    // poll to set error LED if no signal from module
    radioStatus_t getStatus();

    // poll diags to show low-level decoding status and error messages
    void diags();

    // return the count of valid time ticks acquired (each tick is one second, with 59 ticks in a complete message)
    // If the clock is synced, this value will be the number of seconds in the current minute
    // if not synced, its the number of seconds after the last error or last received end-of-minute marker
    uint8_t getTickCount();

    // !! for internal use only. should not be invoked by user application !!
    void signalChange();

protected:
    // error handling
    bool error(int msg, int value);
    void unsetError();

    // called by sub class when all 59 bits of the message have been received
    void syncTime(int secs);
    void syncTimeInternal();

    // conversion helpers
    uint8_t reverse(uint8_t b, uint8_t shift);
    uint8_t bcd2dec(uint8_t bcd);
    uint8_t parity(uint8_t b);
    void dayOfYearToTm(time_t doy, uint8_t leapYear, tmElements_t &tm);

    // FSM triggers
    bool T_1D();
    bool T_2D();
    bool T_3D();
    bool T_5D();
	bool T_8D();
    bool T_1U();
	bool T_2U();
    bool T_5U();
    bool T_7U();
    bool T_8U();
    bool T_9U();
    bool T_18U();
    bool T_19U();
    bool T_18_19U();
    bool T_MESSAGE_COMPLETE();
    bool T_EXIT_60_SECS();

    // print current state
    void printState();
    
    // turns on and off leds to indicate error and sync status
    void setLEDs();

    // following abstract methods must be implemented by sub class

    // execute finite state machine transition, return CONTINUE to next transition or WAIT for signal change
    virtual bool doFSM();

    // read buffer into Time data structure
    virtual void read(tmElements_t &tm);

    // get state as string for diagnostics
    virtual const char *state2Str(uint8_t state);

    // get bit buffer for diagnostics
    virtual uint64_t getBuffer();

    // class fields

    // state fields
    volatile uint8_t fsm_state;            // state of the finite state machine
    volatile int errMsg;                // last error message
    volatile long errValue;                // value to be displayed with error message

    // demodulation
    volatile int state;                 // The led state toggle
    volatile unsigned long tUp;         // time millis() signal up
    volatile unsigned long tDown;       // time millis() signal down
    volatile unsigned long tOldDown;    // time millis() previous signal down
    volatile unsigned long tStartMsg;   // time millis() last start of
    volatile int bitCount;              // input buffer counter
    volatile uint8_t lastIn;            // if last signal was up or down
    volatile unsigned long dDuration;   // duration of last low signal received, 0 if received high signal
    volatile unsigned long uDuration;   // duration of last high signal received, 0 if received low signal
    volatile unsigned long currentTime; // time of signal change
    volatile uint8_t lastBit;           //

    // control
    int _signalPin;                   // digital pin to which the output of the dcf module is attached
    bool _invertedSignal;             // true if the input signal is inverted
    int _signalLEDPin;                // LED pin for signal reception display
    int _errorLEDPin;                 // LED pin for signal reception display
    int _syncedLEDPin;                // lit when signal is synced and turned off on any error 
    int _onPin;                       // pin for turning the receiver on
    int _onLevel;                     // HIGH/LOW level to set on onPin
//    bool started;                     // indicates if receiver was started (replaced by status_stopped)
    volatile bool errorState;         // true if we are in error state
    volatile radioStatus_t status;    // synced, acquiring or error
    int _tzOffset;                    // desired offset in hours from CET
    volatile time_t syncedTime;       // the time of the last sync
    volatile bool syncReady;

};
#endif
