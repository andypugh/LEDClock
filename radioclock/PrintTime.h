/*
 * PrintTime.h
 *
 *  Created on: 14.01.2012
 *      Author: test123
 */

 
#ifndef PRINTTIME_H_
#define PRINTTIME_H_

// utility function for digital clock display: prints preceding colon and leading 0
void printDigits(int digits) {
    if (digits < 10)
        Serial.print('0');
    Serial.print(digits);
}

// display time
void printTime(time_t time) {
	printDigits(hour(time));
    Serial.print(":");
    printDigits(minute(time));
    Serial.print(":");
    printDigits(second(time));
    Serial.print(" ");
    printDigits(day(time));
    Serial.print(".");
    printDigits(month(time));
    Serial.print(".");
    Serial.print(year(time));
    //Serial.println(); // removed by mem 19 jan
}

#endif /* PRINTTIME_H_ */
