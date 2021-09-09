# KAOS
Karl's Arduino Operating Scheme - Blinking lights and stuff configured in EEPROM

## Rationale

Want to have lots of pretty blinky lights, sounds and motors all controlled by your Arduinos? Ever get fed up modifying your code and recompiling everytime
you attach a new device or swap things around? Want to have complete control over all your inputs and remotes? Then KAOS is for you!

Well, strictly speaking not perhaps perfectly for you, but I hope that some of the concepts and ideas might be useful to you in your own projects, so I've
tried to document and explain everything that I have done.

## Underlying Concepts

* Include in-code support for as many devices and features as will fit into the available PROGMEM - unused PROGMEM is just an wasted resource
* Describe the hardware device connections and feature parameters in the EEPROM memory
* Provide a simple serial monitor program to update the EEPROM as required, and offer remote control abilities

