#include <Time.h>
#include <RadioClocks.h>
#include <PrintTime.h>


const int signalPin = 2;
const boolean inverted = false;
const int signalLED = 13;

// uncomment the line that matches the protocol of the connected module
DCF77 radioClock(signalPin, inverted, signalLED);
//MSF60 radioClock(signalPin, inverted, signalLED);
//WWVB  radioClock(signalPin, inverted, signalLED);
//JJY  radioClock(signalPin, inverted, signalLED);

time_t prevDisplay = 0; // time of last display
int oldCount = -1;      // last number of bits logged


// returns the current RadioClock time for synchronization
time_t getRadioClockTime()
{
  return radioClock.getTime();
}

// update the internal time of the Time library
void syncCallback(time_t syncedtime)
{
  // this is called from an intterupt so don't spend too much time here
  setTime(syncedtime);
}

// gets the current time
// returns 0 if we have not been able to sync with RadioClock at least once
time_t getTime()
{
  // getStatus will set the error LED in case of an error
  radioClock.getStatus();
  // time status is calculated in a call to now()
  time_t result = now();
  if (timeStatus()!= timeNotSet)
    return result;
  return 0;
}

// check if count, eg number of bits received, has changed, if yes, print it.
void showCount()
{
	int count = radioClock.getTickCount();
	     if(count != oldCount){
	         Serial.print("count="); Serial.println(count);
	         oldCount = count;
	     }
}

// check if time changed and if time has been syncrhonized at least once, if yes, print it.
void showTime()
{
	  time_t currentTime = getTime();
	  if(currentTime != 0 && currentTime != prevDisplay)
	  {
	       prevDisplay = currentTime;
	       printTime(currentTime);
	       Serial.println();
	  }

}


void setup()
{
  Serial.begin(9600);
  
  // timezone offset is 1 hour
  // radioClock.setTimeZoneOffset(-1);
  
  // set callback for synchronizing Time with RadioClock
  radioClock.setTimeCallback(syncCallback);
  
  // start the Radio Clock
  radioClock.start();
  
  // wait for first sync and log diagnostics
  while(radioClock.getStatus() != status_synced)
  {  
     showCount();
     radioClock.diags();  
  }
  Serial.println("----- ! BRAVO ! WE GOT TIME ! -----------");
  showTime();
}


void loop()
{
  showTime();
  //radioClock.diags();      
  showCount();

  delay(10);	 
}
