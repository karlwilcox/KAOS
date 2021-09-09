# KAOS
Karl's Arduino Operating Scheme - Blinking lights and stuff configured in EEPROM

## Rationale

Want to have lots of pretty blinky lights, sounds and motors all controlled by your Arduinos? Ever get fed up modifying your code and recompiling everytime
you attach a new device or swap things around? Want to have complete control over all your outputs, automated or remote controlled? Then KAOS is for you!

Well, strictly speaking not perhaps perfectly for you, but I hope that some of the concepts and ideas might be useful to you in your own projects, so I've
tried to document and explain everything that I have done.

## Underlying Concepts

* Include in-code support for as many devices and features as will fit into the available PROGMEM - unused PROGMEM is just a wasted resource
* Describe the hardware device connections and feature parameters in the EEPROM memory
* Provide a simple serial monitor program to update the EEPROM as required, and offer remote control abilities

## Program Code

The program code contains support for a number of common devices, both input and output and also various "signal generators" that can be used
to drive your outputs. It may seem to wasteful to have to have code included for devices that are not present but (provided everything fits into 
the available PROGMEM space) this is not a problem. There is no performance penalty arising from the presence of unused code, and we gain a lot of 
flexibility - want to add a motor? Just plug it in, modify the EEPROM data, press reset and away you go... In addition, if you have multiple arduinos
then they can ALL contain the same code, just describe the different hardware setups in the EEPROM data.

## Operating Modes

The basic expectation is that you will turn on your Arduino and all the connected devices, and the system will automatically run a sequence of events:
turning lights on and off, running motors and generally putting on a pretty display. The built-in "signal generators" allow for all sort of effects and
patterns to be generated, including:

* Simple on/off or flashing at fixed or random intervals
* Alternate on/off (e.g. emergency vehicle red/blue lights)
* Flickering lights, fade in/out, cross-fades (best with PWM outputs)
* Sequences (any pattern, e.g. chase, build, random)
* Output groups (common activity across all in the group)
* Actions in response to digital or analog inputs

Alternatively, the automatic processes above can be suspended and devices can be controlled by sending commands over the serial line monitor. Every device
has a unique "tag" and is individually addressable through the serial interface.

## Full Details

Are to be found in the Wiki Pages.

