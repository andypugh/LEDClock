Readme file for the SyncControl class of the Arduino RadioClock library

Please read README.txt before using this class.

This class makes use of the TimeAlarms library which is available here:

http://code.google.com/p/arduino-time/

To save power we may want to sync only at regular intervals
and stop the receiver module when we do not need it.

For example we will want to sync only once in 24 hours or at 3:00 h in the morning.
Otherwise we want to turn the module off and avoid interrupts.

Usage example is provided in the SyncControlExample sketch.

How to use:
-----------

* define a RadioClock object:

    DCF77 radioClock(signalPin, inverted, signalLED);

* define a callback function:

    void syncCallback(time_t syncedtime)
    {
      // this is called from an intterupt so don't spend too much time here
      setTime(syncedtime);
    }
    
* create a SyncControl object:

    SyncControl syncControl(&radioClock, syncCallback, 1, 2, 2);
    
* start it:

    void setup()
    {
      syncControl.start();
    }
    
Note that we do not call radioclock.start(). This is called within SyncControl.
Note that SyncControl makes use of TimeAlarms. This requires polling of Alarm.delay(...).

Available functions:
----------------------

  * SyncControl(RadioClock* radioclock, setTime_t callback, int syncPeriod = 24*60, time_t syncHour = -1, int startSyncCycle = 60, int startSyncDuration = 10);
  
     Constructor
     
     mandatory:
     
     ** radioclock : 
          radioclock signal encoder. SyncControl will call the start() and stop() functions of this object.
     
     ** callback :
        the given callback method will be called to set the system time when the radio clock is synced
        this method is called from within an interrupt, so do not spend too much time here
        SyncControl will call radioclock.setTimeCallback()
        
     optional:
     
     ** syncPeriod :
          duration in minutes to sleep after time has been synced. 
          Default value is 24 hours
          To ignore periodical sync set it to -1.
          
     ** syncHour : 
          the hour of the day at which the module is turned on.
          Default value is -1
          This parameter if used only if syncPeriod is -1
          
     ** startSyncDuration :
          On call to SyncControl.start() the radioclock module is started for the given duration (in minutes)
          Afer this duration the module is stopped.
          Default value: 5 minutes
          To disable set to -1
          
     ** startSyncCycle :
          If after startSyncDuration the radiomodule has not been able to receive the time the module is put to sleep for 
          startSyncCyle. After that it is turned on again for startSyncDuration minutes.
          Default value is 60 minutes
          To disable set to -1
          
  
  * void start();
      Start SyncControl.
      Call this function from within setup()
       
  * void diags();
      poll diags to show status changes


