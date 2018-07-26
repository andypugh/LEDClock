/*
 * DCF77.h - library for DCF77 time
 * This library is intended to be uses with Arduino Time.h library functions
 */

#ifndef DCF77_h
#define DCF77_h

#include "RadioClock.h"

// DCF time format struct
struct DCF77Buffer_
{
     uint32_t prefix    :21;// ignored
     uint8_t Min        :7;    // minutes
     uint8_t P1         :1;    // parity minutes
     uint8_t Hour       :6;    // hours
     uint8_t P2         :1;    // parity hours
     uint8_t Day        :6;    // day
     uint8_t Weekday    :3;    // day of week
     uint8_t Month      :5;    // month
     uint8_t Year       :8;    // year (5 -> 2005)
     uint8_t P3         :1;    // parity
};

union DCF77Buffer {
    struct DCF77Buffer_ data;
    uint64_t bits;
};

// library interface description
class DCF77: public RadioClock {
    // user-accessible "public" interface (see readme.txt for usage)
public:

    DCF77(int signalPin, bool invertedSignal = false, int signalLEDPin = NO_PIN,
          int syncedLEDPin = NO_PIN, int errorLEDPin = NO_PIN, int onPin = NO_PIN, int onLevel = -1);

private:

    bool doFSM();
    void read(tmElements_t &tm);

    bool addBit(uint8_t b1);
    bool parityOk();
    const char * state2Str(byte state);

    volatile union DCF77Buffer buffer; // input buffer

};

#endif

