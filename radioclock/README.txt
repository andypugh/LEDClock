Readme file for the Arduino RadioClock library

The framework implements different radio clock signal decoders.
see:
      http://en.wikipedia.org/wiki/Radio_clock
      
Currently decoders for the following signals have been implemented:

     * DCF77   Germany              http://en.wikipedia.org/wiki/DCF77
     * MSF60   United Kingdom       http://en.wikipedia.org/wiki/Time_from_NPL
     * WWVB    United States        http://en.wikipedia.org/wiki/WWVB
     * JJY     Japan                http://en.wikipedia.org/wiki/JJY

This framework requires the Time framework which can be downloaded here:

      http://arduino.cc/playground/Code/Time
      
Quick Start
-----------

  * Unzip the Time and RadioClock libraries to <arduino ide folder>/libraries/Time and <arduino ide folder>/libraries/RadioClock

  * Start the arduino IDE
  
  * Attach the signal receiver module to the arduino
  
  * In your IDE open File -> Examples -> RadioClock -> Examples -> GenericExamples
  
  * In the GenericExample.pde adapt the constants signalPin, inverted, signalLED to your hardware configuration and uncomment the constructor to match your
    hardware
  
  * Attach the arduino to your PC and load the firmware
  
  * If you attached a signal LED the LED should be blinking approx. once per second.
    If the LED does not blink, then check your hardware
    If the LED blinks irregularly the signal is noisy, try to adjust the antenna direction and remove any radio signal noise sources 
    such as Wifi, Computers, fluorescent bulbs, ... 
  
  * If you have attached an error LED the LED will be turned off after a few minutes, once a complete time message has been received.
    The LED is turned on when an error is detected.
  

The functions available in the library are the following
--------------------------------------------------------

   * RadioClock(int signalPin, bool invertedSignal = false, int signalLEDPin = NO_PIN,
                        int syncedLEDPin = NO_PIN, int errorLEDPin = NO_PIN, int onPin = NO_PIN, int onLevel = -1);
                        
        constructor. Use the constructor of the subclass corresonding to your hardware module.
        
        mandatory:
            ** signalPin 
                  digital input pin attached to module output. must be 2 or 3.
                  
				optional:
				    ** invertedSignal
				          some modules deliver an inverted signal, some require a transistor to amplify the signal, which may invert it.

            ** signalLedPin
                  output pin to a signal LED. When signal is received LED blinks.

            ** errorLEDPin
                  output pin to a signal LED. 
                  LED is turned on if message start has not been detected
                  or an error parsing the message has occured
                  
            ** onPin 
                  some modules have an input for (PON) for turning the receiver on/off. this is the corresponding output pin.
                   
            ** onLevel
                  LOW/HIGH indicates how to set the onPin to turn the receiver on.


   * time_t getTime();
        get the time from the Radio Clock Module. Returns 0 if no time has been received in the past minute or an error has occured
        acuracy of this method is 1 sec.
        For higher accuracy (estimated 50 ms. at sync, but do not take my word on that) use setTimeCallback(...) and Time.getTime()

   * void setTimeCallback( setTime_t callback);
        the given callback method will be called to set the system time when the radio clock is synced
        this method is called from within an interrupt, so do not spend too much time here

   * bool start();
       start the reception. 
       if onPin has been set the receiver is turned on
       attaches the interrupt to signalPin
       returns false if unable to start (caused by invalid pin arguments)

   * void stop();
       stop the reception.
       if onPin has been set the receiver is turned off
       detaches the interrupt to signalPin

   * void setTimeZoneOffset(int hours);
       set offset in hours to the current time zone (for example: uk = -1, when using DCF77)

   * bool isStarted();
       return true if start() was called

   * radioStatus_t getStatus();
       return indicates if time is valid, error or still acquiring
       poll to set error LED if no signal from module

   * void diags();
       poll diags to show low-level decoding status and error messages

   * uint8_t getTickCount();
       return the count of valid time ticks acquired (each tick is one second, with 59 ticks in a complete message)
       ff the clock is synced, this value will be the number of seconds in the current minute
       if not synced, its the number of seconds after the last error or last received end-of-minute marker

Implementation notes
--------------------

The software works in the background within an interrupt routine. Therefore, no invocations in the main loop are required.
However, if no signal is received from the module (for example when power to the module is cut off), the error LED will be turned on, only if getStatus() is invoked 
from a timer interrupt or the main loop.

The decoding is implemented using a Finite State Machine (FSM) (see http://en.wikipedia.org/wiki/Finite-state_machine)
For documentation purposes image files describing the different decoder FSM are included. (created using the open source QM modeling tool http://www.state-machine.com/qm/).

The state is stored in RadioClock.fsm_state field.
Events (Triggers) are implemented as boolean functions.
They are coded as follows: U == signal Up (high), D == signal down (low), 1 == 100 millis etc. For example 9U stands for 900 millis of signal up.
The function doFSM() implements the state transition. This function returns a boolean, indicating if to continue to the next state transision or wait for the next signal level transition.

To log diagnostics messages to the serial port poll the diags() method. See the Diagnostics sketch in the library examples.
To toggle specific diagnostics edit the following definitions in RadioClock.cpp:

  * SHOW_SIGNAL_TIMING_1           low level log. log the duration of signal changes. use when assuming noisy signal. may affect interrupt timing
  * SHOW_SIGNAL_TIMING             low level log. log the of low/high signal. use when assuming issues with the module. may affect interrupt timing
  * SHOW_BITS                      logs decoding of bits within a message
  * SHOW_SIGNAL                    logs duration of low/high signal as used in the FSM.
  * SHOW_FSM_STATE                 logs state transitions of the FSM.

NOTE: if your application makes use of further interrupts, you may have to adjust the timing thresholds

The library provides 2 alternatives to synchronize the internal time of the Time library:

  * use Time.setSyncProvider(provider), where provider calls RadioClock.getTime()
              pro: re-sync intervall can be set in Time
              con: Time polls RadioClock.getTime() until a time is available. The accuracy at sync is 1 sec
  * use RadioClock.setTimeCallback(callback), where callback calls Time.setTime(time)
              pro: synching is done immediatly when a time message has been received. The accuracy at sync is in the millis.
              con: no re-sync intervall
