Modded soldering iron ZD-20U
============================

Alternative firmware for USB powered soldering iron ZD-20U.

![see here](https://github.com/ole00/zd-20u/raw/master/img/inuse.jpg "in use on Medium mode, reduced current & heat")

Requires modification of the internals of the iron.

WORK IN PROGRESS - use at your own risk!

Version 1.0
- uses Attiny13a MCU to set the heat on the soldering tip
- 3 heat modes: Full, Medium, Low and an Off/Sleep state.
- switch between the heat modes by pressing a button (not the flaky metal stud!)
- visual indication of the heat modes via LEDs:
  * Full      : Red + Green
  * Medium    : Red + Dimmed Green
  * Low       : Red
  * Off/Sleep : no LED is on
- heat modes/temperatures are user configurable via heat profile.  Lower value of
  the heat profile means lower temepratures for Medium and Low mode.
  
  The values in are **roughly** (low, medium ,full):
  * Heat 1: °C 200, 250, 330 (°F 390, 480, 625)
  * Heat 2: °C 230, 265, 330 (°F 450, 510, 625)
  * Heat 3: °C 240, 280, 330 (°F 465, 535, 625)
  * Heat 4: °C 250, 290, 330 (°F 480, 555, 625)
  
  Temeprature values depend on many factors like quality / rating of the probe
  and thermometer,  dirt on the soldering tip, size of solder blob on the tip
  and how well it touches the probe, size / mass of the probe, ambient temperature, 
  voltage and current in your battery etc.
- timer to turn off the heat after configurable amount of seconds. 
- timeouts are user configurable via timeout profiles. Lower value of the 
  timeout profile means shorter time to switch to Off/Sleep mode.
  Value 1: 32 seconds, value 2: 64 seconds, value 3: 96 secs, value 4: 128 secs 
- LED indication (blinking) that the heat will be turned off shortly.
  During that time pressing the button snoozes the the heat-off timer.
  The bigger the timeout profile value the longer the indication time. 
  Value 1: 10 seconds, value 2: 12 seconds, value 3: 14 secs, value 4: 16 secs 
- sleep profile allows the iron to completely turn off and cool down (value 1)
  or to get asleep to draw some current every 18 seconds (value 2). Sleeping
  saves some energy while keeping the iron hot. It also tricks some 'smart' power 
  banks not to auto turn off. Default value of the sleep profile is 1 (full off).  
- enter profile selection / configuration  by holding the button pressed
  for 5 seconds right after the power-up
  
  RED blinks R times to indicate which profile is being set:
  * 1 RED blink -> the heat profile
  * 2 RED blinks -> the timeout profile
  * 3 RED blinks -> the sleep profile
  
  After the RED blink, the GREEN led starts to blink. The number of GREEN flashes
  indicates the profile value. After the GREEN flashes the iron expects the user
  to press the button to select the value she desires. If a button is not pressed
  within ~1 second the next value is presented (more GREEN blinks).
  Example:
  <pre>
   1) RED flashes once: heat profile is being configured
   2) user waits until GREEN flashes 3x
   3) user presses the button -> she now selected value 3 of the heat profile
   4) RED flashes twice: timeout profile is being configured
   5) user waits until GREEN flashes 2x
   6) user presses the button -> she now selected value 2 of the timeout profile
   7) RED flashes three times: sleep profile is being configured
   8) user waits until GREEN flashes 1x
   9) user presses the button -> she now selected value 1 of the sleep profile
   10) configuration is done and the iron goes to normal operation mode.
   </pre>

   <pre>
   **************** !!! WARNING  !!!  *****************
   NEVER ENTER THE CONFIGURATION/SETUP MODE WHEN 
   THE IRON IS HOT AS YOU MAY BURN YOURSELF WHILE
   FIDDLING WITH THE PROFILES
   ****************************************************
   </pre>
- current configuration profile is displayed during start-up.
  RED blinks X times to indicate current heat profile
  GREEN blinks Y times to indicate current timeout profile.
  The Sleep profile is not indicated.



I) Openning & stripping off the old components
----------------------------------------------
- remove the transparent tip cap
- unscrew the metal tip bracket
- remove the yellow plastic front bracket
- unscrew the 3 screws, open the stranslucent body
- desolder:
   * diode D1
   * shake detector (little silver tube)
   * yellow wire with the spring
   * SMD capacitor C1 - the small brown one. LEAVE THE BIG BLACK SOLDERED!
   * resistor R3
   * IC1 - the 555 chip

- gently & slowly lift one side of the big black capacitor up
  to about 45 degrees so you can reach the trace underneath (close to the RED LED, trace cut 3)


II) Modifications
====================

PCB modification image: 
![see here](https://github.com/ole00/zd-20u/raw/master/img/mod.jpg "PCB modification image")


1) Cut trace 1 - do continuity test on the test points (as indicated by yellow triangles with #1)
2) Cut trace 2 - optional if you will use hole-through LED and resitor. Do continuity test as indicated by yellow triangles with #2.
3) Cut trace 3 - do continuity test on the test points (as indicated by yellow triangles with #3)
4) Cut trace 4 on the bottom side - do continuity test on the test points (as indicated by yellow triangles with #4)

Note: after the trace cut the continuity tone MUST NOT be audible! Failing to do continuity test
may damage your board and you may get hurt! 

5) solder a wire as pictured by the blue trace. This patch trace connects to +5V.
6) solder a wire as pictured by the blue trace - ensure it doesn't touch other pads around.
  This patch trace connects to GND.
7) Take a break, make a cofee and relax for 20 minutes !
8) Solder ATtiny13a to IC1 pads, ensure the pin 1 is the top left one.


9) Now decide whether you solder hole-throug LED and resistor (option A)
 or SMD LED and resistor (Option B). Check the image for more info.
 
Option A: hole-through components

10) Use small strip of kapton tape to cover the left pad of D1 and also
    to cover the Via left to it (close to the trace cut 1).
11) Solder the LED to the right side pad of the D1 position. Use hot-glue to 
    secure the LED in place.
12) Solder resistor 100 to 300 Ohm between the LED and the trace leading to IC1 pin 2.
    Check the image for details.


Option B: SMD components

11) Solder the LED to position D1. Ensure new trace 6) is still in place and doesn't 
    touch other contacts.
12) Ensure you made the trace cut 2), then solder resitor 100 to 300 Ohm between
    the exposed pads leading to IC pin 2. Check the image for details.
    

Common for all:

13) Solder thin kynar wires leading from the pads of C1. The length of the wires is
    about 7cm (2.5"). Do continuity test and secure the wires with hot glue or small strip of kapton
    tape so you don't accidentally snap them off. Solder the other ends of the wires to the 
    tactile button contacts.

14) Use snippers / wire cutters to remove the little spike from the bottom part of the iron case. The spike is located right under the hole for the button and originally kept the little spring in place.
    Be very gentle not to break the fragile case itself. Make sure the whole spike and its base
    is removed - so that you can glue there the tactile button.
15) Make a visual mark on the side of the bottom case, right where the spike was. This will be the center of the tactile button.
    
16) Use heat gun and drop a hot bead on the spot where the spike was, then quickly glue the tactile
    switch on top of the hot glue bead - ensure your mark matches the center of the button.
17) If you don't have a tactile button with a long 'nose' sticking out of the button hole then use polymorph to create (or 3d print) a little extension for the button.


III) Programming the ATtiny13a
===============================
Use SOIC8 test clamp make wired connection to ATtiny13a. 
Search on ebay for: 'Programmer Testing Clip SOP SOIC 8 SOIC8 DIP8 DIP 8 SOP8 Pin IC Test Clamp'
http://www.ebay.co.uk/itm/Programmer-Testing-Clip-SOP-SOIC-8-SOIC8-DIP8-DIP-8-SOP8-Pin-IC-Test-Clamp-/272258646764
or similar SOIC8 test clamp. Or borrow it from a friend.

- ensure you have your Arduino IDE installed (mine is 1.8.3)
- install John "smeezekitty" ATtiny13a core from here:
  https://github.com/orlv/at13 
  (In Linux) ensure you copy the content from inside the 'at13-master' directory
  to your 'arduino/hardware/' subdirectory. So you will have this path available:
  'arduino/hardware/attiny13/avr/cores/core13/Arduino.h' etc.
- patch the Attiny13a core code so it does not steal the timer interrupt:

  edit hardware/attiny/avr/cores/core13/wiring.c and temporarily change
  <pre>
  ISR(TIM0_OVF_vect)
  to
  ISR(TIMER0_OVF_vect)
  </pre>
 
- connect your Arduino Uno to your PC. 
- open Arduino IDE
- select in menu: File->Examples->Arduino ISP
- select in menu: Tools->Board->Arduino/Genuino Uno
- select in menu: Tools->Programmer->AVRISP mkII
- select in menu: Sketch->Upload
- now your Arduino Uno acts like a Attiny13a chip programmer

- disconnect your Arduino Uno from the PC
- attach the Programmer testing clamp to your Attiny13a (it's OK if it's already soldered to the PCB)
- note the position of pin 1 on the clamp (usually it is the red marked wire)
- connect the 6 wires between the clamp and your Arduino UNO (AU) as on the picture:

![see here](https://github.com/ole00/zd-20u/raw/master/img/arduino_attiny13a.jpg "Arduino and Attiny13a connection")
<pre>
  AU Ground -> Pin 4 on the clamp
  AU 5V     -> Pin 8 on the clamp
  AU dig 10 -> Pin 1 on the clamp
  AU dig 11 -> Pin 5 on the clamp
  AU dig 12 -> Pin 6 on the clamp
  AU dig 13 -> Pin 7 on the clamp
</pre>

- connect your Arduino Uno to your PC. 
- open Arduino IDE
- select in menu: Tools->Board->Attiny13(Attiny13a)
- select in menu: Tools->Programmer->Arduino as ISP
- select in menu: Tools->Frequency->1.2 Mhz
- select in menu: Tools->Burn Bootloader
- open the soldering_iron_zd20u sketch (the sketch from this github repo)
- select in menu: Sketch->Upload
- if you get errors like:
  "avrdude: Yikes!  Invalid device signature."
  then you have not connected the clamp or the wires between Arduino Uno and the clamp properly.
- if you get "Upload done" on the status line, then you are done :-)
- disconnect the clamp from the Atttiny13a and assemble the soldering iron back together

IV) Schematic
=============
![see here](https://github.com/ole00/zd-20u/raw/master/img/schem.png "schematic")

