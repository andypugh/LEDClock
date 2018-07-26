Library for decoding the DCF77 time signal and synchronizing with the arduino Time framework

It has been inspired by code written by Andreas Tacke http://fiendie.net

The code has been tested with an arduino UNO and the following DCF77 modules:

Module 1:
http://www.kundoxt.com/funkuhrtechnik/funkuhrempfaenger/
http://www.reichelt.de/index.html?;ACTION=7;LA=3;OPEN=0;INDEX=0;FILENAME=X200%252FDCF77.pdf;SID=11TttxRn8AAAIAAAlxGu8cefad6f6bd8216c0f4ab8d202892d823

                                 ____ 
                            +---|____|---+
                  T1        |     R3     |
         ____     |/c-------+------------|---o DIGITAL PIN2/3
SIG o---|____|---b|                      | 
          R1      |\e->-----+            |
                            |    ____    |
VDD o-------------------+---|---|____|---+---o +5V
                       _|_  |     R2           
                    C1 ___  |                
                        |   |                
GND o-------------------+---+----------------o GND

C1: 0.1uF Capacitor
R1: 10k Resistor
R2: 300 Resistor
R3: 10k Resistor
T1: 2N2222


Module 2:

http://www.hkw-elektronik.de/pdfenglisch/EM2S-dcfE.pdf

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
