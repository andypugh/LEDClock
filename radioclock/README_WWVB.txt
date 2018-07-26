Library for decoding the WWVB time signal and synchronizing with the arduino Time framework

It has been inspired by code written by Andreas Tacke http://fiendie.net
And by http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1242626686

The decoder has not yet been tested with a receiver module.
If you were able to test it please provide us with details.

The decoder has been tested with a simulator sketch which is provided in the Examples.

The code can bee tested with an arduino UNO and the following WWVB module:

http://www.hkw-elektronik.de/englisch/products/assemblys/receiver_module.php


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

