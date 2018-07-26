/*
 * WWVB.cpp - library for WWVB time
 * This library is intended to be uses with Arduino Time.h library functions
 */

#ifndef WWVB_h
#define WWVB_h

#include "RadioClock.h"


// WWVB time format struct
// other bits received are ignored in addBit(bit)
// Markers are ignored
struct WWVBBuffer_
{
   uint8_t Min          :7;  // minutes
   uint8_t Hour         :6;  // hours
   uint8_t DayOfYearTen :6;  // Jan 1 = 1 .. Dec 31 = 356/366 / 10
   uint8_t DayOfYear    :4;  // Jan 1 = 1 .. Dec 31 = 356/366 % 10
   uint8_t Year         :8;  // year (5 -> 2005)
   uint8_t LYI          :1;  // leap year indicator
};

union WWVBBuffer {
  struct WWVBBuffer_ data;
  uint64_t bits;
};

// library interface description
class WWVB: public RadioClock {
  // user-accessible "public" interface
public:
  // signalLedPin: output pin to a signal LED. When signal is received LED blinks once per second.
  //               set to -1 if not required. 
  // errorLEDPin: output pin to a signal LED. LED is turned on if message start has not been detected
  //              or an error parsing the message has occured .
  //              Set to -1 if not required
  // signalPin: digital input pin attached to module output. must be 2 or 3.
  // inversedSignal: some modules deliver an inversed signal, some require a transistor to amplify the signal, which may invert it.
  // onPin: some modules have an input for (PON) for turning the receiver on/off. this is the corresponding output pin. 
  //       set to -1 if not required
  // onLevel: LOW/HIGH indicates how to set the onPin to turn the receiver on.
  // debug: output debug information using Serial.print
  WWVB(int signalPin, bool invertedSignal = false, int signalLEDPin = NO_PIN,
        int syncedLEDPin = NO_PIN, int errorLEDPin = NO_PIN, int onPin = NO_PIN, int onLevel = -1);

private:

  bool doFSM();
  void read(tmElements_t &tm);

  bool addBit(uint8_t b1);
  bool parityOk();
  const char * state2Str(byte state);
  
  byte bufferCount; // points to next bit to be added to the WWVBBuffer

  volatile union WWVBBuffer buffer; // input buffer

};

#endif

