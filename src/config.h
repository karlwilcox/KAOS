#ifndef CONFIG_H
#define CONFIG_H

#define NANO 1
#define MEGA 0
#define UNO3 0

// echo input over the serial line back to sender
#define FLAG_ECHO           0x01
// Automatically run all outputs as per device actions
#define FLAG_AUTO_OUTPUTS   0x02
// Automatically run all input processing as per device actions
#define FLAG_AUTO_INPUTS    0x04

#define MAX_DEVICES 32


#endif
