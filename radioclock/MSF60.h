/*
 * MSF60.h - library for MSF60 time
 * This library is intended to be uses with Arduino Time.h library functions
 */

#ifndef MSF60_h
#define MSF60_h

#include "RadioClock.h"

// MSF time format struct 
struct MSF60Buffer_ {
    uint32_t prefix :17; // ignored
    uint8_t Year :8;    // year (5 -> 2005)
    uint8_t Month :5;   // month
    uint8_t Day :6;     // day
    uint8_t Weekday :3; // day of week
    uint8_t Hour :6;    // hours
    uint8_t Min :7;     // minutes
    uint8_t STW :2;     // ignored
    uint8_t P1 :1;      // parity minutes
    uint8_t P2 :1;      // parity hours
    uint8_t P3 :1;      // parity
    uint8_t P4 :1;      // parity
    uint8_t ST :1;      // unused
};

union MSF60Buffer {
    struct MSF60Buffer_ data;
    uint64_t bits;
};

// library interface description
class MSF60: public RadioClock {
    // user-accessible "public" interface (see readme.txt for usage)
public:

    MSF60(int signalPin, bool invertedSignal = false, int signalLEDPin = NO_PIN,
            int syncedLEDPin = NO_PIN, int errorLEDPin = NO_PIN, int onPin = NO_PIN, int onLevel = -1);

private:

    bool doFSM();
    void read(tmElements_t &tm);

    bool addBits(uint8_t b1, uint8_t b2);
    bool parityOk();
    const char * state2Str(byte state);

    volatile union MSF60Buffer buffer; // input buffer

};

#endif

