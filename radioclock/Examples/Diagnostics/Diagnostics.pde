/*
 RadioClock diagnostic sketch 
 This version provides examples showing dignosotic capabilities of the library
 Select the desired protocol by uncommenting the version that mathces your radio
*/ 
 
#include <Time.h>
#include <RadioClocks.h>
#include <PrintTime.h>


const int signalPin    = 2;  // pin connected to the radio module output 
const boolean inverted = true; // true if module inverts the output signal
const int signalLED    = 13; // flashes when radio time signal received
const int syncedLEDPin = 6;  // lights when time is synced with radio signal
const int errorLEDPin  = 7;  // lights if there is a decoding error
const int onPin = NO_PIN;    // optional pin to turn on radio module
const int onLevel = LOW;     // the level on the onPin to turn on the module

// uncomment the line that matches the protocol of the connected module
DCF77 radioClock(signalPin,inverted,signalLED,syncedLEDPin,errorLEDPin,onPin,onLevel);
//MSF60 radioClock(signalPin,inverted,signalLED,syncedLEDPin,errorLEDPin,onPin,onLevel);
//WWVB radioClock(signalPin,inverted,signalLED,syncedLEDPin,errorLEDPin,onPin,onLevel);


time_t prevDisplay = 0; // time of last display
time_t prevSync=0;      // time of last sync 

void mySetTime(time_t syncedTime)
{
  // this is called from an interrupt so don't spend too much time here
  setTime(syncedTime);           // this sets the time
  prevSync = syncedTime;         // save the time
  Serial.println("Synced");      // print that the time has been synced 
}

void setup()
{
  Serial.begin(9600);
  
  // optional timezone offset (in hours)
  radioClock.setTimeZoneOffset(-1);       // only needed if radio time not correct for your timezone.
  radioClock.setTimeCallback(mySetTime);  // give the library the function to set the time 
  
  radioClock.start();   // start the Radio Clock
  static int oldCount = -1; 
  Serial.println("Waiting for start message...");
  while(radioClock.getStatus() != status_synced)
  {  
     int count = radioClock.getTickCount();
     if(count != oldCount){
         Serial.print("count = "); Serial.println(count);    
         oldCount = count;
     }       
     radioClock.diags();  
  }
}

void loop()
{
  // check if time changed and if time has been syncrhonized at least once
  time_t currentTime = now();
  if(currentTime != prevDisplay) 
  {
       prevDisplay = currentTime;
       printTime(currentTime);  
       if(radioClock.getStatus() != status_synced)
       {
         Serial.print(" seconds since last sync=");
         Serial.print(currentTime - prevSync);     
       }
       Serial.println();
  }
  radioClock.diags();  // shows any errors
  delay(100);	 
}