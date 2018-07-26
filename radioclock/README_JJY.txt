Library for decoding the MSF60 time signal and synchronizing with the arduino Time framework

It has been inspired by code written by Andreas Tacke http://fiendie.net
And by by code written by Richard Jarkman http://www.jarkman.co.uk/catalog/robots/msftime.htm

The code has been tested with an arduino UNO and the following MSF60 module:

http://www.hkw-elektronik.de/deutsch/produkte/baugruppen/empfangsmodule.php

And the following circuit:
                                ____ 
                           +---|____|---+
                           |     R3     |
                           |            |
SIG o----------------------+------------|---o DIGITAL PIN2/3 
                                        |
VDD o-----------------------------------+---o +5V
                                    
GND o-----------+---------------------------o GND
                |
PON o-----------+


R3: 10k Resistor


These components are provided with arduino starter kit and proto shield (sparkfun)

For state signaling you can add an LED for error signaling, and an LED for blinking on signal receiption.

Some boards support a PON pin for turning the module receiption on or off to reduce power consumption.
Use the start() stop() methods to turn the module on/off.

The module is sensitive to radio noise such as monitors, cell phones, fluorescent lamps, power supplies, etc

