/*
Pulse Clock system for radio time and Arduino
*/

#include <MSF60.h>
#include <Time.h>
#include <RadioClock.h>
#include <PrintTime.h>
#include <TimeAlarms.h>

const int signalPin    = 2;  // pin connected to the radio module output
const boolean inverted = true; // true if module inverts the output signal
const int signalLED    = NO_PIN; // flashes when radio time signal received
const int syncedLEDPin = NO_PIN;  // lights when time is synced with radio signal
const int errorLEDPin  = NO_PIN;  // lights if there is a decoding error
const int onPin = NO_PIN;    // optional pin to turn on radio module
const int onLevel = LOW;     // the level on the onPin to turn on the module

int pulseLength = 150; // length of pulse to clock
int pulseTime = 30; // how many seconds between clock pulses
int pulseWait = 500;


// uncomment the line that matches the protocol of the connected module
//DCF77 radioClock(signalPin,inverted,signalLED,syncedLEDPin,errorLEDPin,onPin,onLevel);
MSF60 radioClock(signalPin,inverted,signalLED,syncedLEDPin,errorLEDPin,onPin,onLevel);
//WWVB radioClock(signalPin,inverted,signalLED,syncedLEDPin,errorLEDPin,onPin,onLevel);


time_t prevSync=0;      // time of last sync

void mySetTime(time_t syncedTime)
{
  // this is called from an interrupt so don't spend too much time here
  if ((syncedTime - prevSync) < 200 || prevSync == 0) { // require two times within 2min before trusting them
    setTime(syncedTime);           // this sets the time
  }
  prevSync = syncedTime;         // save the time
}

void setup()
{
  Serial.begin(9600);

  // optional timezone offset (in hours)
  radioClock.setTimeZoneOffset(0);       // only needed if radio time not correct for your timezone.
  radioClock.setTimeCallback(mySetTime);  // give the library the function to set the time

  radioClock.start();   // start the Radio Clock
  static int oldCount = -1;
  int sig;
  // Radio signal
  pinMode(2, INPUT);
  // 10s of Minutes
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  // Hours
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  // Tens of Hours
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(A5, OUTPUT);
  // Minutes
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  // Blanking
  pinMode(A4, OUTPUT);
  digitalWrite(A4, 1);
  
  Serial.println("Waiting for start message...");
  while(radioClock.getStatus() != status_synced)
  {
     int count = radioClock.getTickCount();
     int d1 = count % 10;
     int d2 = floor(count / 10);
     if(count != oldCount){
         Serial.print("count = "); Serial.println(count);
         oldCount = count;
     }
    sig = digitalRead(2);
     radioClock.diags();
    // 10s of Minutes
    digitalWrite(3, d2 & 1);
    digitalWrite(4, d2 & 2);
    digitalWrite(6, d2 & 4);
    digitalWrite(5, d2 & 8);
    // Hours   
    digitalWrite(7,  0);
    digitalWrite(8,  1);
    digitalWrite(9,  0);
    digitalWrite(10, 1);
    // Tens of Hours
    digitalWrite(11, 0);
    digitalWrite(12, 0);
    digitalWrite(13, 0);
    digitalWrite(A5, sig);
    // Minutes
    digitalWrite(A0, d1 & 1);
    digitalWrite(A1, d1 & 2);
    digitalWrite(A2, d1 & 4);
    digitalWrite(A3, d1 & 8);
    
  }

  Serial.print("Time now - ");
  printTime(prevSync);
  Serial.println();
}

void loop() {
  time_t t = now();
  int m1 = minute(t) % 10;
  int m10 = floor(minute(t) / 10);
  int h1 = hour(t) % 10;
  int h10 = floor(hour(t) / 10);
  Serial.print(hour(t));
  Serial.print(minute(t));
  Serial.print(" h10 ");
  Serial.print(h10);
  Serial.print(" h1 ");
  Serial.print(h1);
  Serial.print(" m10 ");
  Serial.print(m10);
  Serial.print(" m1 ");
  Serial.println(m1);
  // 10s of Minutes  
  digitalWrite(3, m10 & 1);
  digitalWrite(4, m10 & 2);
  digitalWrite(6, m10 & 4);
  digitalWrite(5, m10 & 8);
  // Hours 
  digitalWrite(7,  h1 & 1);
  digitalWrite(8,  h1 & 2);
  digitalWrite(9,  h1 & 4);
  digitalWrite(10, h1 & 8);
  // Tens of Hours
  digitalWrite(A4, 1);  
  digitalWrite(11, h10 & 1);
  digitalWrite(12, h10 & 2);
  digitalWrite(13, h10 & 4);
  digitalWrite(A5, h10 & 8);
  // Minutes
  digitalWrite(A0, m1 & 1);
  digitalWrite(A1, m1 & 2);
  digitalWrite(A2, m1 & 4);
  digitalWrite(A3, m1 & 8);
  
  delay(60000);
}
